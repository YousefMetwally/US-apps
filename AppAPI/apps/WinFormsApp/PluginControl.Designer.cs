namespace WinFormsApp
{
    partial class PluginControl
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label1 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.buttonImage = new System.Windows.Forms.Button();
            this.buttonImageRendered = new System.Windows.Forms.Button();
            this.buttonGenMeas = new System.Windows.Forms.Button();
            this.buttonLogMeas = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(-3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(91, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Sample .NET app";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // pictureBox1
            // 
            this.pictureBox1.Location = new System.Drawing.Point(23, 100);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(640, 360);
            this.pictureBox1.TabIndex = 2;
            this.pictureBox1.TabStop = false;
            // 
            // buttonImage
            // 
            this.buttonImage.Location = new System.Drawing.Point(23, 28);
            this.buttonImage.Name = "buttonImage";
            this.buttonImage.Size = new System.Drawing.Size(107, 23);
            this.buttonImage.TabIndex = 3;
            this.buttonImage.Text = "Load raw image";
            this.buttonImage.UseVisualStyleBackColor = true;
            this.buttonImage.Click += new System.EventHandler(this.buttonImage_Click);
            // 
            // buttonImageRendered
            // 
            this.buttonImageRendered.Location = new System.Drawing.Point(23, 60);
            this.buttonImageRendered.Name = "buttonImageRendered";
            this.buttonImageRendered.Size = new System.Drawing.Size(130, 23);
            this.buttonImageRendered.TabIndex = 3;
            this.buttonImageRendered.Text = "Load rendered image";
            this.buttonImageRendered.UseVisualStyleBackColor = true;
            this.buttonImageRendered.Click += new System.EventHandler(this.buttonImageRendered_Click);
            // 
            // buttonGenMeas
            // 
            this.buttonGenMeas.Location = new System.Drawing.Point(162, 28);
            this.buttonGenMeas.Name = "buttonGenMeas";
            this.buttonGenMeas.Size = new System.Drawing.Size(130, 23);
            this.buttonGenMeas.TabIndex = 4;
            this.buttonGenMeas.Text = "Generate measurement";
            this.buttonGenMeas.UseVisualStyleBackColor = true;
            this.buttonGenMeas.Click += new System.EventHandler(this.buttonGenMeas_Click);
            // 
            // buttonLogMeas
            // 
            this.buttonLogMeas.Location = new System.Drawing.Point(326, 28);
            this.buttonLogMeas.Name = "buttonLogMeas";
            this.buttonLogMeas.Size = new System.Drawing.Size(111, 23);
            this.buttonLogMeas.TabIndex = 5;
            this.buttonLogMeas.Text = "Log measurements";
            this.buttonLogMeas.UseVisualStyleBackColor = true;
            this.buttonLogMeas.Click += new System.EventHandler(this.buttonLogMeas_Click);
            // 
            // PluginControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.MenuHighlight;
            this.Controls.Add(this.buttonLogMeas);
            this.Controls.Add(this.buttonGenMeas);
            this.Controls.Add(this.buttonImage);
            this.Controls.Add(this.buttonImageRendered);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.label1);
            this.Name = "PluginControl";
            this.Size = new System.Drawing.Size(453, 383);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button buttonImage;
        private System.Windows.Forms.Button buttonImageRendered;
        private System.Windows.Forms.Button buttonGenMeas;
        private System.Windows.Forms.Button buttonLogMeas;
    }
}
