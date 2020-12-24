#ifndef KPL_LEDGER_DEVICE_TCP_H
#define KPL_LEDGER_DEVICE_TCP_H

#include <kpl/ledger_device.h>

#include <memory>
#include <vector>

namespace kpl {

class LedgerDeviceTCP: public LedgerDevice
{
public:
  static std::unique_ptr<LedgerDevice> create(const char* Host, uint16_t Port);
  static VecDevices listDevices();

  ~LedgerDeviceTCP() override;

  std::string name() const override; 
  bool connect() override;

protected:
  virtual bool exchange(LedgerAnswerBase& Out,
    uint8_t const* Data, const size_t DataLen,
    unsigned TimeoutMS = 0);

private:
  LedgerDeviceTCP(const char* Host, uint16_t Port);
  void close();

  bool send(uint8_t const* Data, size_t DataLen);
  bool read(uint8_t* Out, size_t OutLen, unsigned TimeoutMS = 0);

  int FD_;
  std::string Host_;
  uint16_t Port_;
};

} // kpl

#endif
