#pragma once

#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>

template<typename SIZE>
struct ResourceSelection
{
	SIZE mCurrent;
	std::set<SIZE> mVisited;		// set is sorted for consistent order while hunting
	SIZE mSelected;			// Hash. -1 now for unselected state.
	int mSelectedPos;		// -1 for unselected state.

	ResourceSelection()
	{
		mCurrent = 0;
		mCurrent--;
		Reset();
	}

	void Reset()
	{
		mSelected = 0;
		mSelected--;
		mSelectedPos = 0;
		mSelectedPos--;
	}

	void RecordIfPartner(SIZE shaderHash, std::set<SIZE>& PartnerShaders)
	{
		if (mCurrent && mCurrent != shaderHash)
			PartnerShaders.insert(mCurrent);
	}
};

// We use this when collecting resource info for ShaderUsage.txt to take a
// snapshot of the resource handle, hash and original hash. We used to just
// save the resource handle, but that was problematic since handles can get
// reused, and so we could record the wrong hash in the ShaderUsage.txt
template<typename Resource>
struct Snapshot
{
	Resource* handle;
	uint32_t hash;
	uint32_t orig_hash;

	Snapshot(Resource* handle, uint32_t hash, uint32_t orig_hash) :
		handle(handle), hash(hash), orig_hash(orig_hash)
	{}
};

template<typename Resource>
static inline bool operator<(const Snapshot<Resource>& lhs, const Snapshot<Resource>& rhs)
{
	if (lhs.orig_hash != rhs.orig_hash)
		return (lhs.orig_hash < rhs.orig_hash);
	if (lhs.hash != rhs.hash)
		return (lhs.hash < rhs.hash);
	return (lhs.handle < rhs.handle);
}

template<typename Resource>
struct InfoData
{
	// All are map or set so that ShaderUsage.txt is sorted - lookup time is O(log N)
	std::map<int, std::set<Snapshot<Resource>>> ResourceRegisters;
	std::set<UINT64> PartnerShaders;
	std::vector<std::set<Snapshot<Resource>>> RenderTargets;
	std::set<Snapshot<Resource>> DepthTargets;
	std::map<int, std::set<Snapshot<Resource>>> UAVs;
};

template<typename SIZE, typename Resource, typename Shader>
struct ShaderResourceSelection : ResourceSelection<SIZE>
{
	Shader* mCurrentHandle;
	std::unordered_map<Shader*, UINT64> mRegistered; // All shaders ever registered
	std::unordered_map<UINT64, Shader*> mPreloaded; // All shaders that were preloaded as .bin
	std::unordered_map<Shader*, Shader*> mOriginal; // When MarkingMode=Original, switch to original
	std::unordered_map<Shader*, Shader*> mZeroed; // When MarkingMode=zero.
	std::map<UINT64, InfoData<Resource>> mInfo;	// map so that ShaderUsage.txt is sorted - lookup time is O(log N)

	ShaderResourceSelection() : ResourceSelection<SIZE>(), mCurrentHandle(0) {}

	void ReleasePreloaded() {
		if (mPreloaded.empty())
			return;

		for (auto i = mPreloaded.begin(); i != mPreloaded.end(); ++i)
			i->second->Release();
		mPreloaded.clear();
	}
};

template<typename SIZE, typename Resource, typename ResourceInfo>
struct TargetResourceSelection : ResourceSelection<SIZE>
{
	Resource* mCurrentHandle;
	std::unordered_map<Resource*, UINT64> mRegistered; // All targets ever registered
	std::map<UINT64, ResourceInfo> mInfo;	// map so that ShaderUsage.txt is sorted - lookup time is O(log N)

	TargetResourceSelection() : ResourceSelection<SIZE>(), mCurrentHandle(0)
	{
	}
};