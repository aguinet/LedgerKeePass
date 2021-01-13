using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Threading.Tasks;

using KeePass.Plugins;
using KeePassLib.Keys;

namespace LedgerKeePass
{
  public sealed class LedgerKeePassExt : Plugin
  {
    private IPluginHost Host_ = null;
    private List<LedgerKeyProvider> Provs_ = new List<LedgerKeyProvider>();

    public override bool Initialize(IPluginHost host)
    {
      Host_ = host;
      List<LedgerDevice> Devs = LedgerDevice.listDevices();

      foreach (LedgerDevice Dev in Devs) {
        LedgerKeyProvider Prov = new LedgerKeyProvider(Dev);
        Provs_.Add(Prov);
        Host_.KeyProviderPool.Add(Prov);
      }
      return true;
    }

    public override void Terminate()
    {
      foreach (LedgerKeyProvider Prov in Provs_) {
        Host_.KeyProviderPool.Remove(Prov);
      }
    }
  }

  public sealed class LedgerKeyProvider : KeyProvider
  {
    private LedgerDevice Dev_;

    public override bool DirectKey
    {
      get { return true; }
    }

    public LedgerKeyProvider(LedgerDevice Dev)
    {
      Dev_ = Dev;
    }

    public override string Name
    {
      get { return "Ledger: " + Dev_.name(); }
    }

    public override byte[] GetKey(KeyProviderQueryContext ctx)
    {
      try {
        KPL K = new KPL(Dev_, 500);
        List<int> Slots = K.getValidSlots(500);
        KeyForm KF = new KeyForm(K, Slots);
        KF.StartPosition = FormStartPosition.CenterParent;
        KF.ShowDialog();
        if (KF.DialogResult != DialogResult.OK) {
          return null;
        }
        return KF.Key;
      }
      catch (KPLException e) {
        showException(e);
        return null;
      }
    }

    public static void showException(KPLException e)
    {
      string Msg = e.Message;
      Msg = Msg[0].ToString().ToUpper() + Msg.Substring(1);
      MessageBox.Show(Msg, "Error while communication with the Ledger device", MessageBoxButtons.OK);
    }
  }

  
  internal class KeyForm : Form
  {
    internal class SlotComboElt
    {
      private readonly int Slot;

      public SlotComboElt(int Slot) {
        this.Slot = Slot;
      }

      public string Display {
        get { return "Slot #" + this.Slot; }
      }

      public int Value {
        get { return this.Slot; }
      }
    }

    public KeyForm(KPL K, List<int> Slots): base()
    {
      this.KPLObj = K;
      InitializeComponent();

      this.AcceptButton = this.btnOpen;
      this.btnOpen.Click += new System.EventHandler(this.openKey);
      this.comboType.SelectedIndexChanged += new System.EventHandler(this.typeChanged);
      this.comboType.Items.Add("Name");

      if (Slots.Count == 0) {
        this.comboSlot.Enabled = false;
      }
      else {
        this.comboType.Items.Add("Slot");
        this.comboSlot.DisplayMember = "Display";
        this.comboSlot.ValueMember = "Value";
        foreach (int Slot in Slots) {
          this.comboSlot.Items.Add(new SlotComboElt(Slot));
        }
        this.comboSlot.SelectedIndex = 0;
      }
      this.comboType.SelectedIndex = 0;
    }

    private void openKey(object sender, EventArgs e) {
      const UInt32 TimeoutKeyMS = 60000; // 60s
      const string PleaseAccept = "Please accept accessing this key on your Ledger device!";

      this.btnOpen.Enabled = false;
      this.btnCancel.Enabled = false;
      this.labelStatus.Text = PleaseAccept;

      var Timer = new System.Windows.Forms.Timer();
      Timer.Interval = 1000;
      UInt32 TimeSpent = 0;
      Timer.Tick += new EventHandler((Object o, EventArgs ea) => {
        TimeSpent += 1;
        UInt32 Rem = TimeoutKeyMS/1000 - TimeSpent;
        this.labelStatus.Text = PleaseAccept + "\r\n(" + Rem + "s remaining)";
      });
      Timer.Start();

      Task<byte[]>.Factory.StartNew(() => {
        if (this.KeyType == 0) {
          // Name
          return this.KPLObj.getKeyFromName(this.DbName, TimeoutKeyMS);
        }
        // Slot
        return this.KPLObj.getKey(this.SelectedSlot, TimeoutKeyMS);
      })
      .ContinueWith(t => {
        Timer.Stop();
        if (t.IsFaulted) {
          this.btnOpen.Enabled = true;
          this.btnCancel.Enabled = true;
          t.Exception.Handle((exc) => {
            if (exc is KPLException) {
              this.labelStatus.Text = "Error: " + exc.Message;
              return true;
            }
            return false;
          });
        }
        else {
          this.labelStatus.Text = "";
          this.key = t.Result;
          this.DialogResult = DialogResult.OK;
        }
      }, TaskScheduler.FromCurrentSynchronizationContext());
    }

    private void typeChanged(object sender, EventArgs e)
    {
      if (this.comboType.SelectedIndex == 0) {
        this.comboSlot.Visible  = false;
        this.textDbName.Visible = true;
      }
      else {
        this.comboSlot.Visible  = true;
        this.textDbName.Visible = false;
      }
    }

    public string DbName {
      get { return this.textDbName.Text; }
    }

    public int KeyType {
      get { return this.comboType.SelectedIndex; }
    }
   
    public byte[] Key {
      get { return this.key; }
    }

    public int SelectedSlot {
      get { return (this.comboSlot.SelectedItem as SlotComboElt).Value; }
    }

    private byte[] key;
    private KPL KPLObj;

    // GUI
    private void InitializeComponent()
    {
      this.flowLayoutPanel1 = new FlowLayoutPanel();
      this.lblKey = new Label();
      this.comboType = new ComboBox();
      this.comboSlot = new ComboBox();
      this.textDbName = new TextBox();
      this.btnOpen = new Button();
      this.btnCancel = new Button();
      this.labelStatus = new Label();
      this.flowLayoutPanel1.SuspendLayout();
      this.SuspendLayout();
      //
      // flowLayoutPanel1
      //
      this.flowLayoutPanel1.Anchor = AnchorStyles.Top;
      this.flowLayoutPanel1.AutoSize = true;
      this.flowLayoutPanel1.AutoSizeMode = AutoSizeMode.GrowAndShrink;
      this.flowLayoutPanel1.Controls.Add(this.lblKey);
      this.flowLayoutPanel1.Controls.Add(this.comboType);
      this.flowLayoutPanel1.Controls.Add(this.comboSlot);
      this.flowLayoutPanel1.Controls.Add(this.textDbName);
      this.flowLayoutPanel1.Location = new System.Drawing.Point(42, 3);
      this.flowLayoutPanel1.Name = "flowLayoutPanel1";
      this.flowLayoutPanel1.Size = new System.Drawing.Size(439, 34);
      this.flowLayoutPanel1.TabIndex = 0;
      //
      // lblKey
      //
      this.lblKey.Anchor = AnchorStyles.Left;
      this.lblKey.AutoSize = true;
      this.lblKey.Location = new System.Drawing.Point(3, 7);
      this.lblKey.Name = "lblKey";
      this.lblKey.Size = new System.Drawing.Size(73, 20);
      this.lblKey.TabIndex = 3;
      this.lblKey.Text = "Key type:";
      //
      // comboType
      //
      this.comboType.Anchor = AnchorStyles.Left;
      this.comboType.FormattingEnabled = true;
      this.comboType.Location = new System.Drawing.Point(82, 3);
      this.comboType.Name = "comboType";
      this.comboType.Size = new System.Drawing.Size(121, 28);
      this.comboType.TabIndex = 0;
      //
      // comboSlot
      //
      this.comboSlot.Anchor = AnchorStyles.Left;
      this.comboSlot.FormattingEnabled = true;
      this.comboSlot.Location = new System.Drawing.Point(209, 3);
      this.comboSlot.Name = "comboSlot";
      this.comboSlot.Size = new System.Drawing.Size(121, 28);
      this.comboSlot.TabIndex = 1;
      //
      // textDbName
      //
      this.textDbName.Anchor = AnchorStyles.Left;
      this.textDbName.Location = new System.Drawing.Point(336, 4);
      this.textDbName.Name = "textDbName";
      this.textDbName.Size = new System.Drawing.Size(100, 26);
      this.textDbName.TabIndex = 2;
      //
      // btnOpen
      //
      this.btnOpen.Anchor = ((AnchorStyles)((AnchorStyles.Bottom | AnchorStyles.Right)));
      this.btnOpen.Location = new System.Drawing.Point(319, 110);
      this.btnOpen.Name = "btnOpen";
      this.btnOpen.Size = new System.Drawing.Size(93, 37);
      this.btnOpen.TabIndex = 1;
      this.btnOpen.Text = "&Open";
      this.btnOpen.UseVisualStyleBackColor = true;
      //
      // btnCancel
      //
      this.btnCancel.Anchor = ((AnchorStyles)((AnchorStyles.Bottom | AnchorStyles.Right)));
      this.btnCancel.DialogResult = DialogResult.Cancel;
      this.btnCancel.Location = new System.Drawing.Point(418, 110);
      this.btnCancel.Name = "btnCancel";
      this.btnCancel.Size = new System.Drawing.Size(93, 37);
      this.btnCancel.TabIndex = 2;
      this.btnCancel.Text = "&Cancel";
      this.btnCancel.UseVisualStyleBackColor = true;
      //
      // labelStatus
      //
      this.labelStatus.Anchor = ((AnchorStyles)(((AnchorStyles.Top | AnchorStyles.Left)
              | AnchorStyles.Right)));
      this.labelStatus.AutoEllipsis = true;
      this.labelStatus.Font = new System.Drawing.Font(Label.DefaultFont, System.Drawing.FontStyle.Bold);
      this.labelStatus.Location = new System.Drawing.Point(12, 54);
      this.labelStatus.Name = "labelStatus";
      this.labelStatus.Size = new System.Drawing.Size(491, 38);
      this.labelStatus.TabIndex = 3;
      this.labelStatus.TextAlign = System.Drawing.ContentAlignment.TopCenter;
      //
      // KeyForm
      //
      this.AcceptButton = this.btnOpen;
      this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
      this.AutoScaleMode = AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(523, 159);
      this.Controls.Add(this.labelStatus);
      this.Controls.Add(this.btnCancel);
      this.Controls.Add(this.btnOpen);
      this.Controls.Add(this.flowLayoutPanel1);
      this.FormBorderStyle = FormBorderStyle.FixedSingle;
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.Name = "KeyForm";
      this.Text = "Ledger device";
      this.flowLayoutPanel1.ResumeLayout(false);
      this.flowLayoutPanel1.PerformLayout();
      this.ResumeLayout(false);
      this.PerformLayout();
    }
    
    private FlowLayoutPanel flowLayoutPanel1;
    private Button btnOpen;
    private Label lblKey;
    private ComboBox comboType;
    private ComboBox comboSlot;
    private TextBox textDbName;
    private Button btnCancel;
    private Label labelStatus;

  }

}
