#ifndef FIB_H_
#define FIB_H_

#define		FIBLEN			sizeof(struct TrieNode)		//size of each Trie Node

#define _NOT_DELETE	0
#define _INSERT 1
#define _DELETE	2
#define _CHANGE	3

#define EMPTYHOP			0							//Trie node that does not have a next hop
#define NOTEXIST            -1							//Trie node that does not exist after US
#define HIGHTBIT			2147483648					//Binary: 10000000000000000000000000000000

#define UNITEDNODE 0			
#define PARTNODE 1		//Participator node
#define SPLITNODE 2			
#define EMPTYNODE 3			

#define  BASIC 1
#define  US 2
#define  ALL 3

#define PREFIXLEN 33 // 32bit ,include '\0'
#define TRACE_READ 100000

#include	<math.h>
#include	<string>
#include	<fstream>
#include	<Windows.h>
#include	<stdint.h>
#include	<stack>
using namespace std;


struct TrieNode
{
	TrieNode  *	parent;					//point to father node
	TrieNode  *	lchild;					//point to the left child(0)
	TrieNode  *	rchild;					//point to the right child(1)
	int			oldPort;	            //original next hop
	int			newPort[2];				//next hops after aggregation
	int			path;
	bool		isAggregated;			//the nodes which participates in aggregation
	unsigned char nodeType;             // node Type,  0: agg,  1: participation,  2: split, 3:empty
};

class CFib
{
public:
	TrieNode * m_pTrie;			//root TrieNode

	int allNodeCount;			//to count all the nodes in original Trie, including empty node
	int oldNodeCount;			//the size of initial routing table
	int newNodeCount;			//the size of routing table after US
	int leafNodeCount;
	int leafDeleteNodecount;

	CFib(void);
	~CFib(void);

	unsigned int * CFib::TrafficRead(char *traffic_file);
	int CFib::lookupOneIP_binary(unsigned int IP);
	int CFib::lookupOneIP_compress(unsigned int IP);

	//clear the Trie
	void CFib::ClearTrie(TrieNode* pTrie);

	//clear the compressed trie
	void CFib::ClearCompressedTrie(TrieNode* pTrie);

	//creat a new TrieNode node
	void CreateNewNode(TrieNode* &pTrie);

	//US algorithm 
	void CompressTrie(bool nodeCountFlag = false);

	//get the total number of nodes in Trie  
	void GetNodeCounts();
	void Pretraversal(TrieNode* pTrie);

	void GetNodeCounts2(int algorithm);
	void Pretraversal2(TrieNode* &pTrie, int algorithm);

	void GetNodeCounts3(int algorithm);
	void Pretraversal3(TrieNode* &pTrie, int algorithm);

	//output the result
	void OutputCompressedTrie(TrieNode* pTrie, string twoHopFile, string oneHopFile);
	void OutputCompressedTrieBin(TrieNode* pTrie, string twoHopFile, string oneHopFile);

	bool IsLeaf(TrieNode * pNode);

private:
	//the first procedure of US: union
	void PassOne(TrieNode* pTrie, bool nodeCountFlag);
	//the second procedure of US: split
	void PassTwo(TrieNode* pTrie, bool nodeCountFlag);

	//get and output all the nexthop in Trie
	void GetTrieHops(TrieNode* pTrie, unsigned int iVal, int iBitLen, ofstream* fout, bool ifTwoHop);

	void FreeTrie(TrieNode *node); //release the mem 

public:

	//convert IP data into binary data
	void IpToBinary(string sIP, char sBin[32]);

	//construct RIB tree from file, and return number of items in rib file
	unsigned int BuildFibFromFile(string sFileName);
	unsigned int BuildFibFromBinFile(string sFileName);

	//add a node in Rib tree
	void AddNode(unsigned long lPrefix, unsigned int iPrefixLen, unsigned int iNextHop);

	void UpdateFromFile(string sIpFile, char algorithmType);
	void UpdateFromFile2(string sIpFile, char algorithmType, ofstream* fout);
	void ConvertUpdateFileIpToBin(string sIpFile, string sBinFile);

	void Normal_InsertNode(char *sPrefix, unsigned int iNextHop);
	int Normal_DeleteNode(char *sPrefix, unsigned int iNextHop);

	int US_InsertNode(char *sPrefix, unsigned int iNextHop);
	int US_DeleteNode(char *sPrefix, unsigned int iNextHop);

	void OutPutNormalRib(TrieNode *pTrie, string oneHopFile);
	void GetOutPutNormalRib(TrieNode* pTrie, unsigned int iVal, int iBitLen, ofstream* fout);
};

#endif /* FIB_H_ */