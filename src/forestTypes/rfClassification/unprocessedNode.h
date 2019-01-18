#ifndef rfunprocessedNode_h
#define rfunprocessedNode_h
#include "fpSplit.h"
#include "../baseUnprocessedNode.h"
#include <vector>
#include <random>
#include <assert.h>

namespace fp{


	template <typename T> //
		class unprocessedNode : public baseUnprocessedNode<T>{
			protected:
				splitInfo<T> bestSplitInfo;
				std::vector<int> featuresToTry;
			public:
				unprocessedNode(int numObsForRoot): baseUnprocessedNode<T>::baseUnprocessedNode(numObsForRoot){
				}

				unprocessedNode(int parentID, int dep, bool isLeft): baseUnprocessedNode<T>::baseUnprocessedNode(parentID, dep, isLeft){
				}

				~unprocessedNode(){}

				inline int returnBestFeature(){
					return bestSplitInfo.returnFeatureNum();
				}

				inline double returnBestImpurity(){
					return bestSplitInfo.returnImpurity();
				}

				inline T returnBestCutValue(){
					return bestSplitInfo.returnSplitValue();
				}

				inline void setBestSplit(splitInfo<T> tempSplit){
					if(tempSplit.returnImpurity() < bestSplitInfo.returnImpurity()){
						bestSplitInfo = tempSplit;
					}
				}

				inline void pickMTRY(){
					for (int i=0; i<fpSingleton::getSingleton().returnNumFeatures(); ++i) featuresToTry.push_back(i);
					std::random_shuffle ( featuresToTry.begin(), featuresToTry.end() );
					featuresToTry.resize(fpSingleton::getSingleton().returnMtry());
				}


				inline void loadFeatureHolder(){
					if(baseUnprocessedNode<T>::obsIndices->useBin()){
						for(int q=0; q<baseUnprocessedNode<T>::obsIndices->returnBinnedSize(); q++){
							fpSingleton::getSingleton().prefetchFeatureVal(featuresToTry.back(),baseUnprocessedNode<T>::obsIndices->returnBinnedInSample(q));
						}

						for(int i =0; i < baseUnprocessedNode<T>::obsIndices->returnBinnedSize(); ++i){
							baseUnprocessedNode<T>::featureHolder[i] = fpSingleton::getSingleton().returnFeatureVal(featuresToTry.back(),baseUnprocessedNode<T>::obsIndices->returnBinnedInSample(i));
						}
					}else{

						for(int q=0; q<baseUnprocessedNode<T>::obsIndices->returnInSampleSize(); q++){
							fpSingleton::getSingleton().prefetchFeatureVal(featuresToTry.back(),baseUnprocessedNode<T>::obsIndices->returnInSample(q));
						}

						for(int i =0; i < baseUnprocessedNode<T>::obsIndices->returnInSampleSize(); ++i){
							baseUnprocessedNode<T>::featureHolder[i] = fpSingleton::getSingleton().returnFeatureVal(featuresToTry.back(),baseUnprocessedNode<T>::obsIndices->returnInSample(i));
						}
					}
				}

				inline void setupNode(){
					pickMTRY();
					baseUnprocessedNode<T>::setHolderSizes();
					baseUnprocessedNode<T>::loadLabelHolder();
					baseUnprocessedNode<T>::setNodeImpurity(baseUnprocessedNode<T>::calculateNodeImpurity());
				}


				inline bool goLeft(const int index){
					T featureVal = fpSingleton::getSingleton().returnFeatureVal(bestSplitInfo.returnFeatureNum(),index);

					if(featureVal <= bestSplitInfo.returnSplitValue()){
						return true;
					}else{
						return false;
					}
				}


				inline void moveDataLeftOrRight(){

					baseUnprocessedNode<T>::leftIndices = new stratifiedInNodeClassIndices();
					baseUnprocessedNode<T>::rightIndices = new stratifiedInNodeClassIndices();

					int lNum =0;
					int rNum =0;
					for (int i=0; i < baseUnprocessedNode<T>::obsIndices->returnInSampleSize();++i){
						if(goLeft(baseUnprocessedNode<T>::obsIndices->returnInSample(i))){
							++lNum;
							baseUnprocessedNode<T>::leftIndices->addIndexToInSamples(baseUnprocessedNode<T>::obsIndices->returnInSample(i));	
						}else{
							++rNum;
							baseUnprocessedNode<T>::rightIndices->addIndexToInSamples(baseUnprocessedNode<T>::obsIndices->returnInSample(i));	
						}
					}

					assert(lNum > 0);
					assert(rNum > 0);

					for (int i=0; i < baseUnprocessedNode<T>::obsIndices->returnOutSampleSize();++i){
						if(goLeft(baseUnprocessedNode<T>::obsIndices->returnInSample(i))){
							baseUnprocessedNode<T>::leftIndices->addIndexToOutSamples(baseUnprocessedNode<T>::obsIndices->returnInSample(i));	
						}else{
							baseUnprocessedNode<T>::rightIndices->addIndexToOutSamples(baseUnprocessedNode<T>::obsIndices->returnInSample(i));	
						}
					}

					delete baseUnprocessedNode<T>::obsIndices;
					baseUnprocessedNode<T>::obsIndices = NULL;
				}


				inline void findBestSplit(){
					//timeLogger logTime;
					fpSplit<T> findSplit(baseUnprocessedNode<T>::labelHolder); //This is done twice
					while(!featuresToTry.empty()){
						//logTime.startGiniTimer();
						loadFeatureHolder();
						//logTime.stopGiniTimer();
						setBestSplit(findSplit.giniSplit(baseUnprocessedNode<T>::featureHolder ,featuresToTry.back()));

						featuresToTry.pop_back();
					}
				}

		}; //unprocessedNode.h
}//namespace fp
#endif //unprocessedNode_h
