using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

/*

# Types
## struct ArrayView
- u64 FileOffset
- u32 ElementSize
- u32 ElementCount

# struct BlobEntry
- u64 FileOffset
- u64 SizeInBytes

# struct Mesh
- BlobEntry Name
- u32 IndexBits
- BlobEntry IndexBuffer
- BlobEntry VertexStream0(position: half4)
- BlobEntry VertexStream1(qtangent:snorm4, uv0, half2)

# struct Renderer
- BlobEntry Name
- Matrix4x4 WorldMatrix
- u32 MeshId

# Struct PackHeader
- u64 HeaderMarker
- u64 Version
- ArrayView<BlobEntry> BlobEntryTable
- ArrayView<Mesh> Meshes



 */


class PackState
{
    //inputs
    public List<Mesh> Meshes = new List<Mesh>();
    public Dictionary<Mesh, uint> Mesh2Index = new Dictionary<Mesh, uint>();
    public List<MeshRenderer> Renderers = new List<MeshRenderer>();
    public List<Texture> Textures = new List<Texture>();
    public Dictionary<Texture, uint> Texture2Index = new Dictionary<Texture, uint>();
    public List<Material> Materials = new List<Material>();
    public Dictionary<Material, uint> Material2Index = new Dictionary<Material, uint>();

    public List<byte[]> BlobEntries = new List<byte[]>();

    //tmp data
    //public System.IO.MemoryStream MemoryStream = new System.IO.MemoryStream();
}

public class OmniPackBuilder
{
    /*
     * data
     */
    public string OutputFilePath;
    public HashSet<GameObject> Roots = new HashSet<GameObject>();

    /*
     * APIs
     */

    [MenuItem("Omni/TryRun %#&a")]
    public static void TryRun()
    {
        System.IO.MemoryStream mstream = new System.IO.MemoryStream();
        mstream.WriteByte(4);
        mstream.WriteByte(5);
    }

#if false
    [MenuItem("Omni/PackCurrentScene")]
    public static void PackCurrentScene()
    { }
#endif

    public void AddGameObjectTree(GameObject treeRoot)
    {
        Roots.Add(treeRoot);
    }
    public void Pack()
    {
        var pstate = new PackState();

        TraverseObjects(pstate);
        SerializeMeshes(pstate);
        SerializeRenderers(pstate);
    }
    /*
     * private implementations
     */
    private void TraverseObjects(PackState pstate)
    {
        uint meshCount = 0;
        foreach (var root in Roots)
        {
            var mrs = root.GetComponentsInChildren<MeshRenderer>();
            foreach (var mr in mrs)
            {
                pstate.Renderers.Add(mr);
                var mesh = mr.GetComponent<MeshFilter>().sharedMesh;
                if (pstate.Mesh2Index.ContainsKey(mesh))
                {
                    pstate.Mesh2Index.Add(mesh, ++meshCount);
                    pstate.Meshes.Add(mesh);
                }
            }
        }
    }
    private void SerializeMeshes(PackState pstate)
    {
        
    }
    private void SerializeRenderers(PackState pstate)
    { }
    private void SerializeTextures(PackState pstate)
    { }


}
