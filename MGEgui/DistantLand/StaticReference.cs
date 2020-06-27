using System.Collections.Generic;
using System.IO;

namespace MGEgui.DistantLand
{
    public class StaticReference
    {

        public string name;
        public float x, y, z;
        public float yaw, pitch, roll;
        public float scale;
        public uint staticID;

        public void SetID(Dictionary<string, Static> StaticsList, Dictionary<string, uint> StaticMap)
        {
            string file = StaticsList[name].mesh;
            if (StaticMap.ContainsKey(file))
            {
                staticID = StaticMap[file];
            }
            else
            {
                staticID = (uint)StaticMap.Count;
                StaticMap[file] = staticID;
            }
        }

        public void Write(BinaryWriter bw)
        {
            bw.Write(staticID);
            bw.Write(x);
            bw.Write(y);
            bw.Write(z);
            bw.Write(yaw);
            bw.Write(pitch);
            bw.Write(roll);
            bw.Write(scale);
        }
    }
}
