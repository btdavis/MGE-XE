using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MGEgui.DistantLand
{
    public class MWPlugin
    {
        public string Name;
        public string ShortName;
        public string FullName;
        public bool ESM;
        public DateTime LoadOrder;
        public CheckState Checked;
        public string Dir;

        public MWPlugin(FileInfo file)
        {
            ShortName = file.Name;
            Name = file.Name;
            FullName = file.FullName.ToLowerInvariant();
            ESM = file.Extension.ToLowerInvariant() == ".esm";
            LoadOrder = file.LastWriteTimeUtc;
            Checked = CheckState.Unchecked;
        }

        public MWPlugin(FileInfo file, string dir)
        {
            ShortName = file.Name;
            Name = file.Name + " > " + dir;
            FullName = file.FullName.ToLowerInvariant();
            Dir = dir;
            ESM = file.Extension.ToLowerInvariant() == ".esm";
            LoadOrder = file.LastWriteTimeUtc;
            Checked = CheckState.Unchecked;
        }

        public MWPlugin(FileInfo file, bool ESM)
        {
            ShortName = file.Name;
            Name = file.Name;
            FullName = file.FullName.ToLowerInvariant();
            this.ESM = ESM;
            LoadOrder = file.LastWriteTimeUtc;
            Checked = CheckState.Unchecked;
        }

        public MWPlugin(FileInfo file, string dir, bool ESM)
        {
            ShortName = file.Name;
            Name = file.Name + " > " + dir;
            FullName = file.FullName.ToLowerInvariant();
            Dir = dir;
            this.ESM = ESM;
            LoadOrder = file.LastWriteTimeUtc;
            Checked = CheckState.Unchecked;
        }
    }
}
