using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.IO;
using System.Threading;
using Microsoft.Xna.Framework.Content;
using WinFormsContentLoading;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using PartStacker.FormComponents;
using PartStacker.Geometry;
using PartStacker.MeshFile;

namespace PartStacker
{
    partial class MainForm : Form
    {
        ModelViewerControl Display3D;
        PartsList PartsList;
        Button Import, Delete, Change, Reload, Start, Preview, Export, CopyMirror;
        NumericUpDown PartQuantity, MinHole, MinimumClearance;
        CheckBox RotateMinBox, EnableSinterbox;
        ProgressBar Progress;
        Label InfoLabel;
        RadioButton None, Cubic, Arbitrary;
        NumericUpDown Clearance, Spacing, Thickness, BWidth;
        NumericUpDown xMin, xMax, yMin, yMax, zMin, zMax;

        PartStacker? Stacker;
        Mesh? LastResult;

        ToolStripMenuItem ImportMenu, ExportMenu;

        public MainForm()
            : base()
        {
            // Set initial size
            Width = 1030;
            Height = 654;

            // Fix size
            MinimumSize = Size;
            MaximumSize = Size;

            // Title text
            Text = "PartStacker 1.0 - Tom van der Zanden";

            // Abort stacking when program is closed
            this.FormClosing += (o, e) => { Stacker?.Stop(); };

            // Menustrip for saving etc.
            MenuStrip menu = new MenuStrip();

            ToolStripMenuItem fileMenu = new ToolStripMenuItem("File");
            ToolStripMenuItem item = new ToolStripMenuItem("New");
            item.Click += NewHandler;
            fileMenu.DropDownItems.Add(item);
            item = new ToolStripMenuItem("Open");
            item.Click += OpenHandler;
            fileMenu.DropDownItems.Add(item);
            item = new ToolStripMenuItem("Save");
            item.Click += SaveHandler;
            fileMenu.DropDownItems.Add(item);
            item = new ToolStripMenuItem("Close");
            item.Click += CloseHandler;
            fileMenu.DropDownItems.Add(item);

            ToolStripMenuItem partMenu = new ToolStripMenuItem("Import/Export");
            ImportMenu = new ToolStripMenuItem("Import parts");
            ImportMenu.Click += ImportHandler;
            partMenu.DropDownItems.Add(ImportMenu);
            ExportMenu = new ToolStripMenuItem("Export result as STL");
            ExportMenu.Enabled = false;
            ExportMenu.Click += ExportHandler;
            partMenu.DropDownItems.Add(ExportMenu);

            ToolStripMenuItem about = new ToolStripMenuItem("Help");
            item = new ToolStripMenuItem("About");
            item.Click += (o, e) => { MessageBox.Show("PartStacker is (c)opyright Tom van der Zanden 2011-2013. It is licensed under the GNU General Public License.", "PartStacker", MessageBoxButtons.OK, MessageBoxIcon.Information); };
            about.DropDownItems.Add(item);
            item = new ToolStripMenuItem("Visit website");
            item.Click += (o, e) => { System.Diagnostics.Process.Start("http://www.tomvanderzanden.nl/partstacker"); };
            about.DropDownItems.Add(item);

            menu.Items.Add(fileMenu);
            menu.Items.Add(partMenu);
            menu.Items.Add(about);
            Controls.Add(menu);

            // Panel for drawing the 3D preview
            Display3D = new ModelViewerControl()
            {
                Location = new Point(0, 24),
                Size = new Size(ClientSize.Height - menu.Height, ClientSize.Height - menu.Height),
                BackColor = Color.FromArgb(40, 50, 120)
            };
            Controls.Add(Display3D);

            // ListView showing the base STL files
            PartsList = new(this, ClientSize.Width, menu.Height);
            PartsList.SelectedIndexChanged += PartSelectHandler; // Todo, make List not visible

            // Buttons for interacting with the list view
            Import = new Button()
            {
                Location = new Point(ClientSize.Width - 400, 270 + menu.Height),
                Size = new Size(88, 25),
                Text = "Import"
            };
            Delete = new Button()
            {
                Location = new Point(ClientSize.Width - 302, 270 + menu.Height),
                Size = new Size(88, 25),
                Text = "Delete",
                Enabled = false
            };
            Change = new Button()
            {
                Location = new Point(ClientSize.Width - 204, 270 + menu.Height),
                Size = new Size(88, 25),
                Text = "Change",
                Enabled = false
            };
            Reload = new Button()
            {
                Location = new Point(ClientSize.Width - 106, 270 + menu.Height),
                Size = new Size(88, 25),
                Text = "Reload"
            };
            Import.Click += ImportHandler;
            Delete.Click += DeleteHandler;
            Change.Click += ChangeHandler;
            Reload.Click += ReloadHandler;
            Controls.Add(Import);
            Controls.Add(Delete);
            Controls.Add(Change);
            Controls.Add(Reload);

            // Label with statistics about all the parts
            InfoLabel = new Label()
            {
                Location = new Point(ClientSize.Width - 400, 300 + menu.Height),
                Width = ClientSize.Width - ClientSize.Height - 40
            };
            Controls.Add(InfoLabel);
            SetText();

            // Progressbar for giving information about the progress of the stacking
            Progress = new ProgressBar()
            {
                Location = new Point(ClientSize.Width - 400, ClientSize.Height - 40),
                Size = new Size(279, 25)
            };
            Controls.Add(Progress);

            Start = new Button()
            {
                Location = new Point(ClientSize.Width - 106, ClientSize.Height - 40),
                Size = new Size(88, 25),
                Text = "Start"
            };
            Start.Click += StartHandler;
            Controls.Add(Start);

            // Checkbox to allow section viewing
            CheckBox section = new CheckBox()
            {
                Location = new Point(ClientSize.Width - 400, ClientSize.Height - 75),
                Text = "Section view:",
                CheckAlign = ContentAlignment.MiddleRight,
                Width = 126
            };
            section.CheckedChanged += (o, e) => { Display3D.Section = section.Checked; };
            Controls.Add(section);

            // Export button
            Export = new Button()
            {
                Size = new Size(170, 30),
                Location = new Point(ClientSize.Width - 195, ClientSize.Height - 80),
                Text = "Export result as STL",
                Enabled = false
            };
            Export.Click += ExportHandler;
            Controls.Add(Export);

            // Minimal clearance size control
            Label caption = new Label()
            {
                Text = "Minimum clearance:",
                Location = Location = new Point(ClientSize.Width - 400, ClientSize.Height - 100),
                Width = 110
            };
            Controls.Add(caption);
            MinimumClearance = new NumericUpDown()
            {
                Minimum = 0.5M,
                Maximum = 2,
                Increment = 0.05M,
                Location = new Point(ClientSize.Width - 288, ClientSize.Height - 102),
                Width = 50,
                Value = 1,
                Enabled = true,
                DecimalPlaces = 2
            };
            Controls.Add(MinimumClearance);

            // Group box that contains controls for editing the parts
            TabControl Tabs = new TabControl()
            {
                Location = new Point(ClientSize.Width - 400, 325 + menu.Height),
                Size = new Size(380, 140),
            };

            Controls.Add(Tabs);

            GroupBox rotations = MakeRotationSelectionBox(out None, out Cubic, out Arbitrary);
            None.Click += RotationHandler;
            Cubic.Click += RotationHandler;
            Arbitrary.Click += RotationHandler;

            TabPage partsTab = MakePartsTab(ref rotations, out PartQuantity, out MinHole, out RotateMinBox, out Preview, out CopyMirror);
            Tabs.TabPages.Add(partsTab);
            PartQuantity.ValueChanged += PartQuantityHandler;
            MinHole.ValueChanged += MinHoleHandler;
            RotateMinBox.CheckedChanged += RotateMinBoxHandler;
            Preview.Click += PreviewHandler;
            CopyMirror.Click += CopyHandler;

            TabPage sinterboxTab = MakeSinterboxTab(out Clearance, out Spacing, out Thickness, out BWidth, out EnableSinterbox);
            Tabs.TabPages.Add(sinterboxTab);

            TabPage boundingBoxTab = MakeBoundingBoxTab(out xMin, out yMin, out zMin, out xMax, out yMax, out zMax);
            Tabs.TabPages.Add(boundingBoxTab);
        }

        private static TabPage MakePartsTab(ref GroupBox rotations, out NumericUpDown PartQuantity, out NumericUpDown MinHole, out CheckBox RotateMinBox, out Button Preview, out Button CopyMirror)
        {
            TabPage tab = new TabPage("Part Settings");

            TableLayoutPanel mainPanel = new TableLayoutPanel()
            {
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                AutoSize = true,
                Dock = DockStyle.Fill,
                RowCount = 2,
                ColumnCount = 1,
                Margin = new Padding(0),
                Padding = new Padding(0, 6, 0, 0),
            };
            mainPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 75));
            mainPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            mainPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            tab.Controls.Add(mainPanel);

            {
                TableLayoutPanel bottomPanel = new TableLayoutPanel()
                {
                    AutoSizeMode = AutoSizeMode.GrowAndShrink,
                    AutoSize = true,
                    Dock = DockStyle.Fill,
                    RowCount = 1,
                    ColumnCount = 3,
                    Margin = new Padding(0),
                    Padding = new Padding(0),
                };
                bottomPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
                bottomPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 26));
                bottomPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 26));
                bottomPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 48));
                mainPanel.Controls.Add(bottomPanel, 0, 1);
                var makeButton = (string text) => new Button()
                {
                    Size = new Size(88, 25),
                    Text = text,
                    Enabled = false,
                    Margin = new Padding(6, 0, 0, 0),
                };
                Preview = makeButton("Preview");
                CopyMirror = makeButton("Mirrored copy");
                bottomPanel.Controls.Add(Preview, 0, 0);
                bottomPanel.Controls.Add(CopyMirror, 1, 0);
            }

            TableLayoutPanel topPanel = new TableLayoutPanel()
            {
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                AutoSize = true,
                Dock = DockStyle.Fill,
                RowCount = 1,
                ColumnCount = 2,
                Margin = new Padding(0),
                Padding = new Padding(0),
            };
            topPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
            topPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 46));
            topPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 54));
            mainPanel.Controls.Add(topPanel, 0, 0);

            topPanel.Controls.Add(rotations, 1, 0);

            {
                TableLayoutPanel leftPanel = new TableLayoutPanel()
                {
                    AutoSizeMode = AutoSizeMode.GrowAndShrink,
                    AutoSize = true,
                    Dock = DockStyle.Fill,
                    RowCount = 3,
                    ColumnCount = 2,
                    Margin = new Padding(0),
                    Padding = new Padding(0),
                };
                leftPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 33));
                leftPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 33));
                leftPanel.RowStyles.Add(new RowStyle(SizeType.Percent, 34));
                leftPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 62));
                leftPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 38));
                topPanel.Controls.Add(leftPanel, 0, 0);

                var makeLabel = (string text) => new Label()
                {
                    Text = text,
                    Anchor = AnchorStyles.Left,
                    Padding = new Padding(3),
                };
                var makeNumericUpDown = (int maximum) => new NumericUpDown()
                {
                    Minimum = 0,
                    Maximum = maximum,
                    Width = 50,
                    Value = 1,
                    Enabled = false,
                    Margin = new Padding(9, 1, 7, 0),
                };

                leftPanel.Controls.Add(makeLabel("Quantity:"), 0, 0);
                PartQuantity = makeNumericUpDown(200);
                leftPanel.Controls.Add(PartQuantity, 1, 0);

                leftPanel.Controls.Add(makeLabel("Minimum hole:"), 0, 1);
                MinHole = makeNumericUpDown(100);
                leftPanel.Controls.Add(MinHole, 1, 1);

                leftPanel.Controls.Add(makeLabel("Minimize box:"), 0, 2);
                RotateMinBox = new CheckBox()
                {
                    Checked = false,
                    Enabled = false,
                    Margin = new Padding(9, 1, 7, 0),
                };
                leftPanel.Controls.Add(RotateMinBox, 1, 2);
            }

            return tab;
        }

        private static GroupBox MakeRotationSelectionBox(out RadioButton None, out RadioButton Cubic, out RadioButton Arbitrary)
        {
            GroupBox box = new GroupBox()
            {
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                AutoSize = true,
                Text = "Part rotations",
            };

            TableLayoutPanel panel = new TableLayoutPanel()
            {
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                AutoSize = true,
                Dock = DockStyle.Fill,
                RowCount = 2,
                ColumnCount = 2,
                Margin = new Padding(0),
                Padding = new Padding(0, 0, 0, 3),
            };
            panel.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            panel.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            box.Controls.Add(panel);

            var makeButton = (string text) => new RadioButton()
            {
                Text = text,
                Enabled = false,
                Padding = new Padding(9, 0, 0, 0),
                Margin = new Padding(0),
                Width = 92,
                Height = 19,
            };

            None = makeButton("None");
            panel.Controls.Add(None, 0, 0);

            Cubic = makeButton("Cubic");
            panel.Controls.Add(Cubic, 0, 1);

            Arbitrary = makeButton("Arbitrary");
            panel.Controls.Add(Arbitrary, 1, 0);

            return box;
        }

        private static TabPage MakeSinterboxTab(out NumericUpDown Clearance, out NumericUpDown Spacing, out NumericUpDown Thickness, out NumericUpDown Width, out CheckBox EnableSinterbox)
        {
            TabPage tab = new TabPage("Sinterbox");

            TableLayoutPanel panel = new TableLayoutPanel()
            {
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                AutoSize = true,
                Dock = DockStyle.Fill,
                RowCount = 4,
                ColumnCount = 4,
                Margin = new Padding(0),
                Padding = new Padding(0, 6, 0, 6),
            };
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            tab.Controls.Add(panel);

            var makeLabel = (string text) => new Label()
            {
                Text = text,
                Anchor = AnchorStyles.Left,
                Padding = new Padding(3),
            };
            var makeNumericUpDown = (double minimum, double maximum, double value, double increment) => new NumericUpDown()
            {
                Minimum = (decimal)minimum,
                Maximum = (decimal)maximum,
                Width = 50,
                Value = (decimal)value,
                Increment = (decimal)increment,
                DecimalPlaces = 1,
                Anchor = AnchorStyles.Right,
                Margin = new Padding(9, 1, 0, 0),
            };

            panel.Controls.Add(makeLabel("Clearance:"), 0, 0);
            Clearance = makeNumericUpDown(0.1, 4, 0.8, 0.1);
            panel.Controls.Add(Clearance, 1, 0);

            panel.Controls.Add(makeLabel("Spacing:"), 0, 1);
            Spacing = makeNumericUpDown(1, 20, 6, 0.5);
            panel.Controls.Add(Spacing, 1, 1);

            panel.Controls.Add(makeLabel("Thickness:"), 0, 2);
            Thickness = makeNumericUpDown(0.1, 4, 0.8, 0.1);
            panel.Controls.Add(Thickness, 1, 2);

            panel.Controls.Add(makeLabel("Width:"), 0, 3);
            Width = makeNumericUpDown(0.1, 4, 1.1, 0.1);
            panel.Controls.Add(Width, 1, 3);

            var caption = new Label()
            {
                Text = "Generate sinterbox:",
                Anchor = AnchorStyles.Left,
                Padding = new Padding(10, 3, 0, 3),
                Width = 120,
            };
            panel.Controls.Add(caption, 2, 0);
            EnableSinterbox = new CheckBox()
            {
                Anchor = AnchorStyles.Left,
                Checked = true
            };
            panel.Controls.Add(EnableSinterbox, 3, 0);

            return tab;
        }

        private static TabPage MakeBoundingBoxTab(out NumericUpDown xMin, out NumericUpDown yMin, out NumericUpDown zMin, out NumericUpDown xMax, out NumericUpDown yMax, out NumericUpDown zMax)
        {
            TabPage tab = new TabPage("Bounding Box");

            TableLayoutPanel panel = new TableLayoutPanel()
            {
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                AutoSize = true,
                Dock = DockStyle.Fill,
                RowCount = 4,
                ColumnCount = 4,
                Margin = new Padding(0),
                Padding = new Padding(0, 6, 0, 6),
            };
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.RowStyles.Add(new RowStyle(SizeType.Percent, 25));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            panel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            tab.Controls.Add(panel);

            var makeLabel = (string text, int leftPadding) => new Label()
            {
                Text = text,
                Anchor = AnchorStyles.Left,
                Padding = new Padding(leftPadding, 3, 3, 3),
            };
            var makeNumericUpDown = (double value, AnchorStyles anchor) => new NumericUpDown()
            {
                Minimum = 10,
                Maximum = 250,
                Width = 50,
                Value = (decimal)value,
                Increment = 1,
                DecimalPlaces = 0,
                Anchor = anchor,
                Margin = new Padding(9, 1, 0, 0),
            };

            panel.Controls.Add(makeLabel("Initial X:", 3), 0, 0);
            xMin = makeNumericUpDown(150, AnchorStyles.Right);
            panel.Controls.Add(xMin, 1, 0);

            panel.Controls.Add(makeLabel("Initial Y:", 3), 0, 1);
            yMin = makeNumericUpDown(150, AnchorStyles.Right);
            panel.Controls.Add(yMin, 1, 1);

            panel.Controls.Add(makeLabel("Initial Z:", 3), 0, 2);
            zMin = makeNumericUpDown(30, AnchorStyles.Right);
            panel.Controls.Add(zMin, 1, 2);

            panel.Controls.Add(makeLabel("Maximum X:", 20), 2, 0);
            xMax = makeNumericUpDown(156, AnchorStyles.None);
            panel.Controls.Add(xMax, 3, 0);

            panel.Controls.Add(makeLabel("Maximum Y:", 20), 2, 1);
            yMax = makeNumericUpDown(156, AnchorStyles.None);
            panel.Controls.Add(yMax, 3, 1);

            panel.Controls.Add(makeLabel("Maximum Z:", 20), 2, 2);
            zMax = makeNumericUpDown(90, AnchorStyles.None);
            panel.Controls.Add(zMax, 3, 2);

            return tab;
        }

        public void CopyHandler(object o, EventArgs ea)
        {
            PartsList.MirrorCopySelectedItem();
        }

        public void NewHandler(object o, EventArgs ea)
        {
            PartsList.RemoveAll();
        }

        public void OpenHandler(object o, EventArgs ea)
        {
            OpenFileDialog select = new OpenFileDialog()
            {
                Title = "Load stacking settings from file",
                Filter = "Stacking settings (*.stk)|*.stk"
            };
            DialogResult dr = select.ShowDialog();

            if (dr != DialogResult.OK)
                return;

            try
            {
                Stream stream = File.Open(select.FileName, FileMode.Open);
#pragma warning disable SYSLIB0011
                BinaryFormatter bformatter = new BinaryFormatter();
#pragma warning restore SYSLIB0011

                List<PartsListItem> items = (List<PartsListItem>)bformatter.Deserialize(stream);
                stream.Close();

                PartsList.RemoveAll();

                foreach (PartsListItem item in items)
                    PartsList.Add(item);

                SetText();
            }
            catch
            {
                MessageBox.Show("Error reading file " + select.FileName + "!", "File error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void SaveHandler(object o, EventArgs ea)
        {
            SaveFileDialog select = new SaveFileDialog()
            {
                Title = "Select file to save stacking settings",
                Filter = "Stacking settings (*.stk)|*.stk"
            };
            DialogResult dr = select.ShowDialog();

            if (dr != DialogResult.OK)
                return;

            try
            {
                Stream stream = File.Open(select.FileName, FileMode.Create);
#pragma warning disable SYSLIB0011
                BinaryFormatter bformatter = new BinaryFormatter();
#pragma warning restore SYSLIB0011

                List<PartsListItem> temp = PartsList.AllParts();
                bformatter.Serialize(stream, temp);
                stream.Close();
            }
            catch
            {
                MessageBox.Show("Error writing to file " + select.FileName + "!", "File error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void CloseHandler(object o, EventArgs ea)
        {
            this.Close();
        }

        public void ExportHandler(object o, EventArgs ea)
        {
            SaveFileDialog select = new SaveFileDialog()
            {
                Title = "Select file to save result to",
                Filter = "STL files (*.stl)|*.stl"
            };
            DialogResult dr = select.ShowDialog();

            if (dr != DialogResult.OK)
                return;

            try
            {
                STL.To(LastResult, select.FileName);
            }
            catch
            {
                MessageBox.Show("Error writing to file " + select.FileName + "!", "File error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void RotationHandler(object o, EventArgs ea)
        {
            if (!None.Enabled || !Cubic.Enabled || !Arbitrary.Enabled)
                return;

            ref int index = ref PartsList.SelectedItem.Properties.RotationIndex;
            if (None.Checked)
                index = 0;
            else if (Cubic.Checked)
                index = 1;
            else
                index = 2;
        }

        public void PartQuantityHandler(object o, EventArgs ea)
        {
            if (!PartQuantity.Enabled)
                return;

            PartsList.SelectedItem.SetQuantity((int)PartQuantity.Value);

            SetText();
        }

        public void MinHoleHandler(object o, EventArgs ea)
        {
            if (!MinHole.Enabled)
                return;

            PartsList.SelectedItem.Properties.MinHole = (int)MinHole.Value;
        }

        public void RotateMinBoxHandler(object o, EventArgs ea)
        {
            if (!RotateMinBox.Enabled)
                return;

            PartsList.SelectedItem.Properties.RotateMinBox = RotateMinBox.Checked;
        }

        public void PreviewHandler(object o, EventArgs ea)
        {
            Mesh mesh = PartsList.SelectedItem.Properties.BaseMesh;
            int[,,] voxels_temp = new int[mesh.box.Item1, mesh.box.Item2, mesh.box.Item3];
            int volume = mesh.Voxelize(voxels_temp, 1, (int)MinHole.Value);
            Display3D.SetMeshWithVoxels(mesh, voxels_temp, volume);
        }

        public void ImportHandler(object o, EventArgs ea)
        {
            FileDialog select = new OpenFileDialog()
            {
                Title = "Select model files to import",
                Filter = "STL files (*.stl)|*.stl|All files (*.*)|*.*",
                Multiselect = true
            };

            DialogResult result = select.ShowDialog();
            if (result != DialogResult.OK)
                return;

            for(int i = 0; i < select.FileNames.Length; i++)
            {
                try
                {
                    PartsListItem p = new PartsListItem(select.FileNames[i], STL.From(select.FileNames[i]));
                    
                    PartsList.Add(p);
                    if (select.FileNames.Length == 1)
                    {
                        PartsList.ClearSelection();
                        p.Selected = true;
                    }
                }
                catch
                {
                    MessageBox.Show("Error reading file " + select.FileNames[i] + "!", "File error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            SetText();
        }
        public void DeleteHandler(object o, EventArgs ea)
        {
            int selectedCount = PartsList.SelectedItemCount;
            string messageFragment = selectedCount == 1 ? "this part" : $"these {selectedCount} parts";
            DialogResult confirm = MessageBox.Show($"Are you sure you want to remove {messageFragment}?", "Confirm delete", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            if (confirm != DialogResult.Yes)
                return;

            PartsList.RemoveSelectedItems();

            SetText();
        }
        public void ChangeHandler(object o, EventArgs ea)
        {
            PartsList.SelectedItem.ChangeFile();
            PartSelectHandler(null, null);

            SetText();
        }
        public void ReloadHandler(object o, EventArgs ea)
        {
            PartsList.ReloadSelectedItems();
            SetText();
        }

        private void DisableButtons()
        {
            Start.Text = "Stop";
            PartsList.ClearSelection();
            this.Import.Enabled = false;
            this.ImportMenu.Enabled = false;
            this.Export.Enabled = false;
            this.ExportMenu.Enabled = false;
            this.Reload.Enabled = false;
            this.PartsList.Enable(false);
            this.MinimumClearance.Enabled = false;
        }
        private void EnableButtons()
        {
            Start.Text = "Start";
            this.Import.Enabled = true;
            this.ImportMenu.Enabled = true;
            this.Reload.Enabled = true;
            this.PartsList.Enable(true);
            this.MinimumClearance.Enabled = true;
        }

        public void SetProgress(double progress, double total)
        {
            var func = () => Progress.Value = (int)(100 * progress / total);
            if (Progress.InvokeRequired)
                Invoke(func);
            else
                func();
        }
        public void StartHandler(object o, EventArgs ea)
        {
            bool running = Stacker?.Running ?? false;
            if (!running)
            {
                var (_, modelTriangles, _) = PartsList.Totals();

                if (modelTriangles == 0)
                    return;
                else if (modelTriangles > 1000000)
                {
                    DialogResult confirm = MessageBox.Show("The finished model will exceed 1.000.000 triangles. Are you sure you want to continue?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                    if (confirm != DialogResult.Yes)
                        return;
                }

                DisableButtons();

                PartStacker.Parameters parameters = new()
                {
                    InitialTriangles = modelTriangles + 2,
                    Parts = PartsList.AllParts().Select(part => part.Properties.Clone()).ToArray(),

                    SetProgress = SetProgress,
                    FinishStacking = (bool b, Mesh m) => this.Invoke(() => FinishStacking(b, m)),
                    DisplayMesh = (Mesh m, int x, int y, int z) => this.Invoke(() => { Display3D.SetMesh(m); Display3D.BB = new Microsoft.Xna.Framework.Vector3(x, y, z); }),
                    Resolution = (double)MinimumClearance.Value,

                    xMin = (double)xMin.Value, xMax = (double)xMax.Value,
                    yMin = (double)yMin.Value, yMax = (double)yMax.Value,
                    zMin = (double)zMin.Value, zMax = (double)zMax.Value,
                };

                Stacker = new(parameters);
            }
            else
            {
                DialogResult confirm = MessageBox.Show("Are you sure you want to abort stacking? Any progress will be lost.", "Stop stacking", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (confirm != DialogResult.Yes)
                    return;

                Stacker?.Stop();
                Stacker = null;

                EnableButtons();
                Progress.Value = 0;
            }
        }
        public void FinishStacking(bool succeeded, Mesh result)
        {
            if (succeeded)
            {
                LastResult = result;
                result.CalcBox();
                if (EnableSinterbox.Checked)
                {
                    Sinterbox.Parameters parameters = new()
                    {
                        Clearance = (double)Clearance.Value,
                        Thickness = (double)Thickness.Value,
                        Width = (double)BWidth.Value,
                        Spacing = ((double)Spacing.Value) + 0.00013759,
                    };
                    result.AddSinterbox(parameters);
                    result.CalcBox();
                }
                Display3D.SetMesh(result);
                this.Export.Enabled = true;
                this.ExportMenu.Enabled = true;
                DialogResult dl = MessageBox.Show("Done stacking! Final bounding box: " + Math.Round(result.size.X, 1) + "x" + Math.Round(result.size.Y, 1) + "x" + Math.Round(result.size.Z, 1) + "mm (" + Math.Round(100 * result.Volume() / (result.size.X * result.size.Y * result.size.Z), 1) + "% density).\n\nWould you like to save the result now?", "Stacking complete", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (dl == DialogResult.Yes)
                    ExportHandler(null, null);
            }
            else
            {
                MessageBox.Show("Did not manage to stack parts within maximum bounding box", "Stacking failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            EnableButtons();
            Progress.Value = 0;
        }

        public void PartSelectHandler(object o, EventArgs ea)
        {
            Change.Enabled = false;
            Delete.Enabled = false;
            MinHole.Enabled = false;
            PartQuantity.Enabled = false;
            RotateMinBox.Enabled = false;
            Preview.Enabled = false;
            CopyMirror.Enabled = false;

            None.Enabled = false;
            Cubic.Enabled = false;
            Arbitrary.Enabled = false;

            if (PartsList.SelectedItemCount == 1)
            {
                PartsListItem part = PartsList.SelectedItem;

                Change.Enabled = true;
                if(part.Properties.BaseMesh != null)
                    Display3D.SetMesh(part.Properties.BaseMesh);

                MinHole.Value = part.Properties.MinHole;
                MinHole.Enabled = true;

                PartQuantity.Value = part.Properties.Quantity;
                PartQuantity.Enabled = true;

                RotateMinBox.Checked = part.Properties.RotateMinBox;
                RotateMinBox.Enabled = true;

                CopyMirror.Enabled = true;

                if(part.Properties.BaseMesh != null)
                    Preview.Enabled = true;

                if (part.Properties.RotationIndex == 0)
                    None.Checked = true;
                else if (part.Properties.RotationIndex == 1)
                    Cubic.Checked = true;
                else
                    Arbitrary.Checked = true;

                None.Enabled = true;
                Cubic.Enabled = true;
                Arbitrary.Enabled = true;
            }
            if (PartsList.SelectedItemCount > 0)
                Delete.Enabled = true;
        }

        public void SetText()
        {
            var (parts, triangles, volume) = PartsList.Totals();
            InfoLabel.Text = "Parts: " + parts + " - Volume: " + Math.Round(volume / 1000, 1) + " - Triangles: " + triangles;
        }
    }
}
