using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Windows.Forms;

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
      const UInt32 TimeoutKeyMS = 60000; // 60s
      try {
        KPL K = new KPL(Dev_, 500);
        List<int> Slots = K.getValidSlots(500);
        KeyForm KF = new KeyForm(Slots);
        KF.StartPosition = FormStartPosition.CenterParent;
        KF.ShowDialog();
        if (KF.DialogResult != DialogResult.OK) {
          return null;
        }
        if (KF.KeyType == 0) {
          // Name
          return K.getKeyFromName(KF.DbName, TimeoutKeyMS);
        }
        // Slot
        return K.getKey(KF.SelectedSlot, TimeoutKeyMS);
      }
      catch (KPLException e) {
        string Msg = e.Message;
        Msg = Msg[0].ToString().ToUpper() + Msg.Substring(1);
        MessageBox.Show(Msg, "Error while communication with the Ledger device", MessageBoxButtons.OK);
        return null;
      }
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

    public KeyForm(List<int> Slots): base()
    {
      InitializeComponent();

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

    private void InitializeComponent()
    {
      this.flowLayoutPanel1 = new FlowLayoutPanel();
      this.lblKey = new Label();
      this.comboType = new ComboBox();
      this.comboSlot = new ComboBox();
      this.textDbName = new TextBox();
      this.btnOpen = new Button();
      this.btnCancel = new Button();
      this.flowLayoutPanel1.SuspendLayout();
      this.SuspendLayout();
      // 
      // flowLayoutPanel1
      // 
      this.flowLayoutPanel1.Anchor = AnchorStyles.None;
      this.flowLayoutPanel1.Controls.Add(this.lblKey);
      this.flowLayoutPanel1.Controls.Add(this.comboType);
      this.flowLayoutPanel1.Controls.Add(this.comboSlot);
      this.flowLayoutPanel1.Controls.Add(this.textDbName);
      this.flowLayoutPanel1.Location = new System.Drawing.Point(5, 1);
      this.flowLayoutPanel1.Name = "flowLayoutPanel1";
      this.flowLayoutPanel1.Size = new System.Drawing.Size(444, 37);
      this.flowLayoutPanel1.TabIndex = 0;
      // 
      // lblKey
      // 
      this.lblKey.Anchor = AnchorStyles.Left;
      this.lblKey.AutoSize = true;
      this.lblKey.Location = new System.Drawing.Point(3, 7);
      this.lblKey.Name = "lblKey";
      this.lblKey.Size = new System.Drawing.Size(74, 20);
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
      this.comboType.DropDownStyle = ComboBoxStyle.DropDownList;
      // 
      // comboSlot
      // 
      this.comboSlot.Anchor = AnchorStyles.Left;
      this.comboSlot.FormattingEnabled = true;
      this.comboSlot.Location = new System.Drawing.Point(209, 3);
      this.comboSlot.Name = "comboSlot";
      this.comboSlot.Size = new System.Drawing.Size(121, 28);
      this.comboSlot.TabIndex = 1;
      this.comboSlot.DropDownStyle = ComboBoxStyle.DropDownList;
      // 
      // textDbName
      // 
      this.textDbName.Anchor = AnchorStyles.Left;
      this.textDbName.Location = new System.Drawing.Point(336, 4);
      this.textDbName.Name = "textDbName";
      this.textDbName.Size = new System.Drawing.Size(200, 26);
      this.textDbName.TabIndex = 2;
      // 
      // btnOpen
      // 
      this.btnOpen.Anchor = ((AnchorStyles)((AnchorStyles.Bottom | AnchorStyles.Right)));
      this.btnOpen.DialogResult = DialogResult.OK;
      this.btnOpen.Location = new System.Drawing.Point(265, 71);
      this.btnOpen.Name = "btnOpen";
      this.btnOpen.Size = new System.Drawing.Size(93, 37);
      this.btnOpen.TabIndex = 1;
      this.btnOpen.Text = "&Open";
      this.btnOpen.UseVisualStyleBackColor = true;
      this.AcceptButton = this.btnOpen;
      // 
      // btnCancel
      // 
      this.btnCancel.Anchor = ((AnchorStyles)((AnchorStyles.Bottom | AnchorStyles.Right)));
      this.btnCancel.DialogResult = DialogResult.Cancel;
      this.btnCancel.Location = new System.Drawing.Point(364, 71);
      this.btnCancel.Name = "btnCancel";
      this.btnCancel.Size = new System.Drawing.Size(93, 37);
      this.btnCancel.TabIndex = 2;
      this.btnCancel.Text = "&Cancel";
      this.btnCancel.UseVisualStyleBackColor = true;
      // 
      // KeyForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
      this.AutoScaleMode = AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(469, 120);
      this.Controls.Add(this.btnCancel);
      this.Controls.Add(this.btnOpen);
      this.Controls.Add(this.flowLayoutPanel1);
      this.Name = "KeyForm";
      this.Text = "Ledger device";
      this.flowLayoutPanel1.ResumeLayout(false);
      this.flowLayoutPanel1.PerformLayout();
      this.ResumeLayout(false);
    }

    public string DbName {
      get { return this.textDbName.Text; }
    }

    public int KeyType {
      get { return this.comboType.SelectedIndex; }
    }

    public int SelectedSlot {
      get { return (this.comboSlot.SelectedItem as SlotComboElt).Value; }
    }

    private FlowLayoutPanel flowLayoutPanel1;
    private Button btnOpen;
    private Label lblKey;
    private ComboBox comboType;
    private ComboBox comboSlot;
    private TextBox textDbName;
    private Button btnCancel;
  }

}
