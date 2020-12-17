#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_PRP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_PRP_RP_HH__

//add header from related replacement policy if there
#include "mem/cache/replacement_policies/base.hh"

struct PRPParams;

//for now inherit from base replacement policy
class PRP : public BaseReplacementPolicy
{
  protected:
    /** PRP-specific implementation of replacement data. */
    struct PRPReplData : ReplacementData
    {
        /*ML - Tick on which the entry was last touched. */
        Tick lastTouchTick;
        /** Number of references to this entry since it was reset. */
        unsigned refCount;

        // int blockAge; //T_L in the paper

        float finalBlkProbability;
        
        /**
         * Default constructor. Invalidate data.
         */
        PRPReplData() : lastTouchTick(0), refCount(0), finalBlkProbability(0) 
        {            
        }
        
    };
    
  public:
    
    typedef PRPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    PRP(const Params *p);
    /**
     * Destructor.
     */
    ~PRP() {}

 /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                              const override;

    /**
     * Touch an entry to update its replacement data.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using access timestamps.
     *
     * @param cands Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_PRP_RP_HH__