using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MGEgui.DistantLand
{
    public class MWPlugins
    {
        public Dictionary<string, MWPlugin> Plugins;
        public List<string> Dirs;
        public string DataFiles;

        public MWPlugins(string main, List<string> dirs)
        {
            DirectoryInfo dir = new DirectoryInfo(main);
            DataFiles = dir.FullName.ToLowerInvariant();
            while (dirs.IndexOf(DataFiles) != -1)
            {
                dirs.Remove(DataFiles);
            }
            FileInfo[] files;
            Plugins = new Dictionary<string, MWPlugin>();
            Dirs = new List<string>(dirs);
            files = dir.GetFiles("*.esm");
            foreach (FileInfo file in files)
            {
                Plugins.Add(file.Name.ToLowerInvariant(), new MWPlugin(file, true));
            }
            files = dir.GetFiles("*.esp");
            foreach (FileInfo file in files)
            {
                Plugins.Add(file.Name.ToLowerInvariant(), new MWPlugin(file, false));
            }
            List<string> removeDirs = new List<string>();
            foreach (string dirName in Dirs)
            {
                if (!addPluginsFromDir(dirName))
                {
                    removeDirs.Add(dirName);
                }
            }
            foreach (string dirName in removeDirs)
            {
                dirs.Remove(dirName);
            }
        }

        public void SetDirs(List<string> dirs)
        {
            foreach (string dirName in Dirs)
            {
                if (dirs.IndexOf(dirName) == -1)
                {
                    List<string> entries = new List<string>();
                    foreach (KeyValuePair<string, MWPlugin> entry in Plugins)
                    {
                        if (entry.Value.Dir != null && string.Equals(dirName, entry.Value.Dir, StringComparison.OrdinalIgnoreCase))
                        {
                            entries.Add(entry.Key);
                        }
                    }
                    foreach (string entry in entries)
                    {
                        Plugins.Remove(entry);
                    }
                }
            }
            List<string> removeDirs = new List<string>();
            foreach (string dirName in dirs)
            {
                if (Dirs.IndexOf(dirName) == -1)
                {
                    if (!addPluginsFromDir(dirName))
                    {
                        removeDirs.Add(dirName);
                    }
                }
            }
            foreach (string dirName in removeDirs)
            {
                dirs.Remove(dirName);
            }
            Dirs.Clear();
            Dirs.AddRange(dirs);
        }

        private bool addPluginsFromDir(string srcDir)
        {
            DirectoryInfo dir = new DirectoryInfo(srcDir);
            string casefoldDir = srcDir.ToLowerInvariant();
            if (dir.Exists)
            {
                FileInfo[] files = dir.GetFiles("*.esm");
                foreach (FileInfo file in files)
                {
                    Plugins.Add(file.Name.ToLowerInvariant() + " > " + casefoldDir, new MWPlugin(file, srcDir, true));
                }
                files = dir.GetFiles("*.esp");
                foreach (FileInfo file in files)
                {
                    Plugins.Add(file.Name.ToLowerInvariant() + " > " + casefoldDir, new MWPlugin(file, srcDir, false));
                }
                return true;
            }
            return false;
        }

        public KeyValuePair<string, MWPlugin>[] ByName
        {
            get
            {
                List<KeyValuePair<string, MWPlugin>> temp = new List<KeyValuePair<string, MWPlugin>>(Plugins);
                temp.Sort(delegate (KeyValuePair<string, MWPlugin> firstPair, KeyValuePair<string, MWPlugin> nextPair) {
                    return string.Compare(firstPair.Value.ShortName, nextPair.Value.ShortName, StringComparison.CurrentCulture);
                });
                return temp.ToArray();
            }
        }

        public KeyValuePair<string, MWPlugin>[] ByType
        {
            get
            {
                List<KeyValuePair<string, MWPlugin>> temp = new List<KeyValuePair<string, MWPlugin>>(Plugins);
                temp.Sort(delegate (KeyValuePair<string, MWPlugin> firstPair, KeyValuePair<string, MWPlugin> nextPair) {
                    if (firstPair.Value.ESM != nextPair.Value.ESM)
                    {
                        return firstPair.Value.ESM ? -1 : 1;
                    }
                    if (firstPair.Value.Dir != nextPair.Value.Dir)
                    {
                        return string.Compare(firstPair.Value.FullName, nextPair.Value.FullName, StringComparison.CurrentCulture);
                    }
                    return string.Compare(firstPair.Value.ShortName, nextPair.Value.ShortName, StringComparison.CurrentCulture);
                });
                return temp.ToArray();
            }
        }

        public KeyValuePair<string, MWPlugin>[] ByLoadOrder
        {
            get
            {
                List<KeyValuePair<string, MWPlugin>> temp = new List<KeyValuePair<string, MWPlugin>>(Plugins);
                temp.Sort(delegate (KeyValuePair<string, MWPlugin> firstPair, KeyValuePair<string, MWPlugin> nextPair) {
                    if (firstPair.Value.ESM != nextPair.Value.ESM)
                    {
                        return firstPair.Value.ESM ? -1 : 1;
                    }
                    return firstPair.Value.LoadOrder.CompareTo(nextPair.Value.LoadOrder);
                });
                return temp.ToArray();
            }
        }

        public string[] getSelected()
        {
            List<string> s = new List<string>();
            foreach (KeyValuePair<string, MWPlugin> temp in Plugins)
            {
                if (temp.Value.Checked == CheckState.Checked)
                {
                    s.Add(temp.Key);
                }
            }
            return s.ToArray();
        }
    }
}
