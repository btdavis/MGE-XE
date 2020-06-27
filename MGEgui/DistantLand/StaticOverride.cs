using System;

namespace MGEgui.DistantLand
{
    public struct StaticOverride
    {
        public bool Ignore;
        public StaticType Type;
        public bool OldSimplify;
        public bool overrideSimplify;
        public float Simplify;
        public float Density;
        public bool StaticsOnly;
        public bool NoScript;
        public bool NamesNoIgnore;

        public StaticOverride(byte value)
        {
            OldSimplify = false;
            Simplify = 1;
            Density = -1;
            overrideSimplify = false;
            Ignore = false;
            StaticsOnly = false;
            Type = StaticType.Auto;
            NoScript = false;
            NamesNoIgnore = false;
            switch (value)
            {
                case 0:
                    Ignore = true;
                    break;
                case 1:
                    Type = StaticType.Near;
                    break;
                case 2:
                    Type = StaticType.Far;
                    break;
                case 3:
                    break;
                case 4:
                    overrideSimplify = true;
                    Type = StaticType.Near;
                    break;
                case 5:
                    overrideSimplify = true;
                    Type = StaticType.Far;
                    break;
                case 6:
                    overrideSimplify = true;
                    break;
            }
        }

        public StaticOverride(string value)
        {
            Ignore = false;
            Type = StaticType.Auto;
            OldSimplify = false;
            overrideSimplify = false;
            Simplify = 1;
            Density = -1;
            StaticsOnly = false;
            NoScript = false;
            NamesNoIgnore = false;
            string[] keys = value.ToLowerInvariant().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
            foreach (string s in keys)
            {
                if (s == "ignore")
                {
                    Ignore = true;
                }
                else if (s == "near")
                {
                    Type = StaticType.Near;
                }
                else if (s == "auto")
                {
                    Type = StaticType.Auto;
                }
                else if (s == "far")
                {
                    Type = StaticType.Far;
                }
                else if (s == "very_far")
                {
                    Type = StaticType.VeryFar;
                }
                else if (s.StartsWith("grass"))
                {
                    float percent;
                    Type = StaticType.Grass;
                    if (s.Length > 6 && float.TryParse(s.Remove(0, 6), out percent) && percent >= 0)
                    {
                        if (percent > 100)
                        {
                            Density = 1.0f;
                        }
                        else
                        {
                            Density = percent / 100.0f;
                        }
                    }
                }
                else if (s == "tree")
                {
                    Type = StaticType.Tree;
                }
                else if (s == "building")
                {
                    Type = StaticType.Building;
                }
                else if (s == "no_script")
                {
                    NoScript = true;
                }
                else if (s == "use_old_reduction")
                {
                    OldSimplify = true;
                }
                else if (s.StartsWith("reduction_"))
                {
                    float percent;
                    if (float.TryParse(s.Remove(0, 10), out percent) && percent >= 0 && percent <= 100)
                    {
                        overrideSimplify = true;
                        Simplify = percent / 100.0f;
                    }
                }
                else if (s == "static_only")
                {
                    StaticsOnly = true;
                }
            }
        }

        public StaticOverride(StaticOverride value, bool enabledInNames)
        {
            OldSimplify = value.OldSimplify;
            Simplify = value.Simplify;
            Density = value.Density;
            overrideSimplify = value.overrideSimplify;
            Ignore = value.Ignore;
            StaticsOnly = value.StaticsOnly;
            Type = value.Type;
            NoScript = value.NoScript;
            NamesNoIgnore = enabledInNames;
        }
    }
}
