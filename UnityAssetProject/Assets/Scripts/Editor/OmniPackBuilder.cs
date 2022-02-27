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
- BlobEntry IndexBuffer
- BlobEntry VertexStream0(position: half4)
- BlobEntry VertexStream1(qtangent:snorm4, uv0, half2)

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
	public List<MeshRenderer> Renderers = new List<MeshRenderer>();
	public List<Texture> Textures = new List<Texture>();
	public List<Material> Materials = new List<Material>();

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
	}
	private void TraverseObjects(PackState pstate)
	{
        int meshCount = 0;
        var mht = new Dictionary<Mesh, int>();
        foreach (var root in Roots)
		{
			var mrs = root.GetComponentsInChildren<MeshRenderer>();
			foreach (var mr in mrs)
			{
                pstate.Renderers.Add(mr);
				var mesh = mr.GetComponent<MeshFilter>().sharedMesh;
				if (mht.ContainsKey(mesh))
				{
					mht.Add(mesh, ++meshCount);
                    pstate.Meshes.Add(mesh);
				}
			}
		}
	}
    private void SerializeMeshes(PackState pstate)
	{
		
	}

	/*
	 * private implementations
	 */
}
