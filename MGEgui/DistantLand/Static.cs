namespace MGEgui.DistantLand
{
    public class Static
    {
        public readonly string name;
        public readonly string mesh;
        public float size;

        public Static(string Name, string Mesh)
        {
            name = Name.ToLower(Statics.Culture);
            mesh = Mesh.ToLower(Statics.Culture);
            size = 0;
        }
    }
}
