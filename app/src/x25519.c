#include "x25519.h"
#include "os.h"
#include <kpl/kpl_csts.h>
#include <string.h>

static const uint8_t CURVE25519_G[X25519_PTSIZE] = {9};

static void reverse_bytes(uint8_t *first, uint8_t *last) {
  while ((first != last) && (first != --last)) {
    uint8_t tmp = *last;
    *last = *first;
    *(first++) = tmp;
  }
}

static bool X25519(uint8_t *out, uint8_t const *pub_point,
                   uint8_t const *priv_scalar) {
  uint8_t scalar_[X25519_SCALARSIZE];
  uint8_t outtmp[X25519_PTSIZE + 1];
  bool ret = true;

  // Apply DJB's recommandation on scalar, and then change its endianess,
  // because X25519 uses little endian integers, whereas
  // cx_ecfp_scalar_mult expects big endian ones.
  os_memcpy(scalar_, priv_scalar, sizeof(scalar_));
  scalar_[0] &= 0xf8u;
  scalar_[31] &= 0x7fu;
  scalar_[31] |= 0x40u;
  reverse_bytes(&scalar_[0], &scalar_[32]);

  outtmp[0] = 0x02; // uncompressed point
  CCASSERT(1, X25519_SCALARSIZE == X25519_PTSIZE);
  os_memcpy(&outtmp[1], pub_point, X25519_PTSIZE);
  // Same as above, we also need to reverse the endianess of the point.
  reverse_bytes(&outtmp[1], &outtmp[X25519_PTSIZE + 1]);
  if (cx_ecfp_scalar_mult(CX_CURVE_Curve25519, outtmp, sizeof(outtmp), scalar_,
                          sizeof(scalar_)) == 0) {
    ret = false;
    goto end;
  }
  reverse_bytes(&outtmp[1], &outtmp[X25519_PTSIZE + 1]);
  os_memcpy(out, &outtmp[1], X25519_PTSIZE);

end:
  // Cleanup
  explicit_bzero(outtmp, sizeof(outtmp));
  explicit_bzero(scalar_, sizeof(scalar_));
  return ret;
}

bool X25519EncryptKPKey(uint8_t *kpkey, uint8_t *own_pubkey,
                        uint8_t const *caller_pubkey) {
  // First, generate an ephemeral key pair
  uint8_t own_privkey[X25519_SCALARSIZE];
  uint8_t ecdhe_secret[X25519_PTSIZE];
  uint8_t keystream[CX_SHA256_SIZE];
  bool ret = true;

  cx_rng(own_privkey, sizeof(own_privkey));
  if (!X25519(own_pubkey, CURVE25519_G, own_privkey)) {
    ret = false;
    goto end;
  }

  // Then, compute ECDHE using the caller's public key
  if (!X25519(ecdhe_secret, caller_pubkey, own_privkey)) {
    ret = false;
    goto end;
  }

  // Hash the result alongside both public keys using Blake2B (supported in
  // libsodium). This will be our keystream.
  cx_blake2b_t h;
  cx_blake2b_init(&h, KPL_KEY_SIZE * 8);
  cx_hash((cx_hash_t *)&h, 0, &ecdhe_secret[0], sizeof(ecdhe_secret), NULL, 0);
  cx_hash((cx_hash_t *)&h, 0, caller_pubkey, X25519_PTSIZE, NULL, 0);
  cx_hash((cx_hash_t *)&h, CX_LAST, own_pubkey, X25519_PTSIZE, keystream,
          sizeof(keystream));

  // Finally, xor the key with our ephemeral keystream
  for (size_t i = 0; i < KPL_KEY_SIZE; ++i) {
    kpkey[i] ^= keystream[i];
  }

end:
  // Cleanup
  explicit_bzero(&own_privkey, sizeof(own_privkey));
  explicit_bzero(ecdhe_secret, sizeof(ecdhe_secret));
  explicit_bzero(keystream, sizeof(keystream));
  explicit_bzero(&h, sizeof(h));
  return ret;
}
