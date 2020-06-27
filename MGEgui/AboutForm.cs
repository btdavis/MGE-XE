using System.Drawing;
using System.Windows.Forms;

namespace MGEgui
{
    public class AboutForm : Form
    {
        private System.Timers.Timer OpacityTimer;
        private System.ComponentModel.IContainer components = null;
        delegate void SetOpacityCallback(double opacity);
        public AboutForm(string message, string title, string close)
        {
            this.SuspendLayout();
            // bClose
            Button bClose = new Button();
            bClose.Anchor = AnchorStyles.Bottom;
            bClose.DialogResult = DialogResult.Cancel;
            bClose.Location = new Point(194, 143);
            bClose.Size = new Size(128, 25);
            bClose.TabIndex = 0;
            bClose.UseVisualStyleBackColor = true;
            bClose.Text = close;
            // lBody
            Label lBody = new Label();
            lBody.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            lBody.AutoEllipsis = true;
            lBody.BorderStyle = BorderStyle.Fixed3D;
            lBody.Location = new Point(116, 9);
            lBody.Size = new Size(292, 131);
            lBody.TextAlign = ContentAlignment.MiddleCenter;
            lBody.UseMnemonic = false;
            lBody.Text = message;
            // this
            this.AcceptButton = bClose;
            this.AutoScaleDimensions = new SizeF(96F, 96F);
            this.AutoScaleMode = AutoScaleMode.Dpi;
            this.AutoSize = true;
            this.CancelButton = bClose;
            this.ClientSize = new Size(420, 180);
            this.Controls.Add(lBody);
            this.Controls.Add(bClose);
            this.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Opacity = 0;
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = FormStartPosition.CenterScreen;
            this.Text = title;
            this.ResumeLayout(false);
            // alphaTimer
            this.OpacityTimer = new System.Timers.Timer();
            this.OpacityTimer.Elapsed += new System.Timers.ElapsedEventHandler(IncreaseOpacityEvent);
            this.OpacityTimer.Interval = 25;
            this.OpacityTimer.Start();
        }
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }
        protected override void OnPaint(PaintEventArgs e)
        {
            Graphics graphics = e.Graphics;
            Icon ic = new Icon(Properties.Resources.AppIcon, 96, 96);
            graphics.DrawIcon(ic, 12, 24);
            graphics.Dispose();
        }
        private void SetOpacity(double opacity)
        {
            if (this.InvokeRequired)
            {
                SetOpacityCallback d = new SetOpacityCallback(SetOpacity);
                this.Invoke(d, new object[] { opacity });
            }
            else
            {
                this.Opacity = opacity;
            }
        }
        private void IncreaseOpacityEvent(object source, System.Timers.ElapsedEventArgs e)
        {
            if (this.Opacity < 0.9)
            {
                this.SetOpacity(this.Opacity + 0.03);
            }
            else
            {
                OpacityTimer.Elapsed -= IncreaseOpacityEvent;
            }
        }
    }
}
