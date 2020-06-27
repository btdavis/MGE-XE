namespace MGEgui.DistantLand
{
    public struct StaticToRemove
    {
        public readonly string worldspace;
        public readonly string reference;

        public StaticToRemove(string Worldspace, string Reference)
        {
            worldspace = Worldspace;
            reference = Reference;
        }
    }
}
