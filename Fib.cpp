#include "Fib.h"
#include <iostream>
#include <iomanip>

//#define DEBUG

CFib::CFib(void)
{
	//initial the root of the Trie
	CreateNewNode(m_pTrie);

	allNodeCount = 0;		//to count all the nodes in Trie, including empty nodes
	oldNodeCount = 0;		//to count all the solid nodes in original Trie
	newNodeCount = 0;       //to count all the solid nodes in Trie after US
}

CFib::~CFib(void)
{
	TrieNode *node = m_pTrie;
	if (node != NULL)
	{
		FreeTrie(node);
	}
}

void CFib::FreeTrie(TrieNode *node)
{
	if (node != NULL)
	{
		if (node->lchild != NULL)
			FreeTrie(node->lchild);

		if (node->rchild != NULL)
			FreeTrie(node->rchild);

		free(node);
	}
}

//clear the Fib Trie
void CFib::ClearTrie(TrieNode* pTrie)
{
	if (NULL == pTrie) return;

	ClearTrie(pTrie->lchild);
	ClearTrie(pTrie->rchild);

	if (pTrie->lchild != NULL)
	{
		free(pTrie->lchild);
		pTrie->lchild = NULL;
	}

	if (pTrie->rchild != NULL)
	{
		free(pTrie->rchild);
		pTrie->rchild = NULL;
	}
}

//clear the compressed Trie
void CFib::ClearCompressedTrie(TrieNode* pTrie)
{
	if (NULL == pTrie) return;

	pTrie->newPort[0] = EMPTYHOP;
	pTrie->newPort[1] = EMPTYHOP;
	pTrie->isAggregated = false;
	pTrie->nodeType = -1; //nodeType
	ClearCompressedTrie(pTrie->lchild);
	ClearCompressedTrie(pTrie->rchild);
}

//creat new node 
void CFib::CreateNewNode(TrieNode* &pTrie)
{
	pTrie = (struct TrieNode*)malloc(FIBLEN);

	//initial
	pTrie->parent = NULL;
	pTrie->lchild = NULL;
	pTrie->rchild = NULL;
	pTrie->oldPort = EMPTYHOP;
	pTrie->newPort[0] = EMPTYHOP;
	pTrie->newPort[1] = EMPTYHOP;
	pTrie->isAggregated = false;
	pTrie->nodeType = EMPTYNODE;
	pTrie->path = 0;
}

void CFib::CompressTrie(bool nodeCountFlag)
{
	PassOne(m_pTrie, nodeCountFlag);
	PassTwo(m_pTrie, nodeCountFlag);
}

//unite children's next hops to parent
void CFib::PassOne(TrieNode* pTrie, bool nodeCountFlag = false)
{
	if (IsLeaf(pTrie)) return;

	if (pTrie->lchild != NULL)
		PassOne(pTrie->lchild, nodeCountFlag);
	if (pTrie->rchild != NULL)
		PassOne(pTrie->rchild, nodeCountFlag);

	if (pTrie->lchild != NULL && pTrie->rchild != NULL && !pTrie->lchild->isAggregated && !pTrie->rchild->isAggregated && pTrie->lchild->oldPort != EMPTYHOP && pTrie->rchild->oldPort != EMPTYHOP)
	{//both left and right child can be aggregated
		pTrie->newPort[0] = pTrie->lchild->oldPort;
		pTrie->newPort[1] = pTrie->rchild->oldPort;
		pTrie->isAggregated = true;
		pTrie->lchild->isAggregated = true;
		pTrie->rchild->isAggregated = true;

		pTrie->nodeType = UNITEDNODE; 
		pTrie->lchild->nodeType = PARTNODE; 
		pTrie->rchild->nodeType = PARTNODE; 

		if (IsLeaf(pTrie->lchild))
		{//delete left child
			pTrie->lchild->newPort[0] = NOTEXIST;
			pTrie->lchild->newPort[1] = NOTEXIST;
			if (nodeCountFlag == true)
			{
				free(pTrie->lchild);
				pTrie->lchild = NULL;
			}
		}

		if (IsLeaf(pTrie->rchild))
		{//delete right child
			pTrie->rchild->newPort[0] = NOTEXIST;
			pTrie->rchild->newPort[1] = NOTEXIST;
			if (nodeCountFlag == true)
			{
				free(pTrie->rchild);
				pTrie->rchild = NULL;
			}
		}
	}
	else if (pTrie->lchild == NULL || pTrie->lchild->isAggregated || pTrie->lchild->oldPort == EMPTYHOP)
	{//left child cannot be aggregated
		if (pTrie->oldPort != EMPTYHOP && pTrie->rchild != NULL && !pTrie->rchild->isAggregated && pTrie->rchild->oldPort != EMPTYHOP)
		{//right child can be aggregated
			pTrie->newPort[0] = pTrie->oldPort;
			pTrie->newPort[1] = pTrie->rchild->oldPort;
			pTrie->isAggregated = true;
			pTrie->rchild->isAggregated = true;

			pTrie->nodeType = UNITEDNODE;
			pTrie->rchild->nodeType = PARTNODE;

			if (IsLeaf(pTrie->rchild))
			{//delete right child
				pTrie->rchild->newPort[0] = NOTEXIST;
				pTrie->rchild->newPort[1] = NOTEXIST;
				if (nodeCountFlag == true)
				{
					free(pTrie->rchild);
					pTrie->rchild = NULL;
				}
			}
		}
	}
	else if (pTrie->rchild == NULL || pTrie->rchild->isAggregated || pTrie->rchild->oldPort == EMPTYHOP)
	{//right child cannot be aggregated
		if (pTrie->oldPort != EMPTYHOP && pTrie->lchild != NULL && !pTrie->lchild->isAggregated && pTrie->lchild->oldPort != EMPTYHOP)
		{//left child can be aggregated
			pTrie->newPort[0] = pTrie->lchild->oldPort;
			pTrie->newPort[1] = pTrie->oldPort;
			pTrie->isAggregated = true;
			pTrie->lchild->isAggregated = true;

			pTrie->nodeType = UNITEDNODE;
			pTrie->lchild->nodeType = PARTNODE;

			if (IsLeaf(pTrie->lchild))
			{//delete left child
				pTrie->lchild->newPort[0] = NOTEXIST;
				pTrie->lchild->newPort[1] = NOTEXIST;
				if (nodeCountFlag == true)
				{
					free(pTrie->lchild);
					pTrie->lchild = NULL;
				}
			}
		}
	}
}

//split the nodes which are not united
void CFib::PassTwo(TrieNode* pTrie, bool nodeCountFlag = false)
{
	if (pTrie == NULL) return;

	if (!pTrie->isAggregated && pTrie->oldPort != EMPTYHOP && pTrie->newPort[0] != NOTEXIST)
	{
		pTrie->newPort[0] = pTrie->oldPort;
		pTrie->newPort[1] = pTrie->oldPort;
		pTrie->nodeType = SPLITNODE;
	}

	if (pTrie->lchild != NULL) PassTwo(pTrie->lchild, nodeCountFlag);
	if (pTrie->rchild != NULL) PassTwo(pTrie->rchild, nodeCountFlag);
}


bool CFib::IsLeaf(TrieNode * pNode)
{
	if (NULL == pNode)
		return false;
	if (pNode->lchild == NULL && pNode->rchild == NULL) return true;
	else return false;
}

void CFib::GetNodeCounts()
{
	allNodeCount = 0;
	oldNodeCount = 0;
	newNodeCount = 0;
	leafNodeCount = 0;
	leafDeleteNodecount = 0;
	Pretraversal(m_pTrie);
}

void CFib::Pretraversal(TrieNode* pTrie)
{
	if (NULL == pTrie) return;

	allNodeCount++;
	if (pTrie->oldPort != EMPTYHOP)
		oldNodeCount++;
	if (pTrie->newPort[0] != NOTEXIST && (pTrie->newPort[0] != EMPTYHOP || pTrie->newPort[1] != EMPTYHOP))
		newNodeCount++;

	if (IsLeaf(pTrie))
	{
		leafNodeCount++;
	}
	else
	{
		Pretraversal(pTrie->lchild);
		Pretraversal(pTrie->rchild);
	}
}

void CFib::GetNodeCounts2(int algorithm)
{
	allNodeCount = 0;
	oldNodeCount = 0;
	newNodeCount = 0;
	leafNodeCount = 0;
	leafDeleteNodecount = 0;
	Pretraversal2(m_pTrie, algorithm);
}


void CFib::Pretraversal2(TrieNode* &pTrie, int algorithm)
{
	if (NULL == pTrie) return;

	allNodeCount++;

	if (algorithm == BASIC)
	{
		if (pTrie->oldPort != EMPTYHOP)
			oldNodeCount++;
	}
	else if (algorithm == US)
	{
		if (pTrie->nodeType == UNITEDNODE || pTrie->nodeType == SPLITNODE)
			oldNodeCount++;
	}

	if (algorithm == BASIC)
	{
		if (IsLeaf(pTrie))
		{
			leafNodeCount++;
			//delete the node		
			TrieNode *parent = pTrie->parent;
			TrieNode *tmp = pTrie;
			pTrie = NULL;
			int flag = 0;
			while (IsLeaf(tmp))
			{
				if (NULL == parent)
					break;
				if (flag == 1 && tmp->oldPort != EMPTYHOP)
					break;

				if (parent->lchild && parent->lchild == tmp)
				{
					parent->lchild = NULL;
				}
				else if (parent->rchild && parent->rchild == tmp)
				{
					parent->rchild = NULL;
				}

				leafDeleteNodecount++;

				free(tmp);
				tmp = parent;
				parent = parent->parent;
				flag = 1;
			}
		}
		else
		{
			if (pTrie->lchild && pTrie->rchild)
			{
				Pretraversal2(pTrie->lchild, algorithm);
				Pretraversal2(pTrie->rchild, algorithm);
			}
			else if (pTrie->lchild)
			{
				Pretraversal2(pTrie->lchild, algorithm);
			}
			else
			{
				Pretraversal2(pTrie->rchild, algorithm);
			}
		}
	}// Nomal algorithm
	else if (algorithm == US)
	{
		if (IsLeaf(pTrie)) // cmp leaf node, change to trimmed trie
		{
			leafNodeCount++;
			TrieNode *parent;
			TrieNode *tmp;

			parent = pTrie->parent;
			tmp = pTrie;
			int flag = 0;
			while (IsLeaf(tmp))
			{
				if (NULL == parent)
					break;

				if (flag == 1 && (tmp->nodeType == SPLITNODE || tmp->nodeType == UNITEDNODE)) // get the latest nonempty nodes 
				{
					break;
				}
				if (parent->lchild && parent->lchild == tmp)
				{
					parent->lchild = NULL;
				}
				else if (parent->rchild && parent->rchild == tmp)
				{
					parent->rchild = NULL;
				}

				leafDeleteNodecount++;
				free(tmp);
				tmp = parent;
				parent = parent->parent;
				flag = 1;
			}
		}
		else
		{
			if (pTrie->lchild && pTrie->rchild)
			{
				Pretraversal2(pTrie->lchild, algorithm);
				Pretraversal2(pTrie->rchild, algorithm);
			}
			else if (pTrie->lchild)
			{
				Pretraversal2(pTrie->lchild, algorithm);
			}
			else
			{
				Pretraversal2(pTrie->rchild, algorithm);
			}
		}
	}//US
}


void CFib::GetNodeCounts3(int algorithm)
{
	allNodeCount = 0;
	Pretraversal3(m_pTrie, algorithm);
}

void CFib::Pretraversal3(TrieNode* &pTrie, int algorithm)
{
	if (NULL == pTrie) return;

	if (algorithm == BASIC)
	{
		if (pTrie->oldPort != EMPTYHOP)
			allNodeCount++;
	}
	else if (algorithm == US)
	{
		if (pTrie->nodeType == UNITEDNODE || pTrie->nodeType == SPLITNODE)
			allNodeCount++;
	}

	if (pTrie->lchild)
		Pretraversal3(pTrie->lchild, algorithm);
	if (pTrie->rchild)
		Pretraversal3(pTrie->rchild, algorithm);
}

void CFib::OutputCompressedTrie(TrieNode* pTrie, string twoHopFile, string oneHopFile)
{
	ofstream fout(twoHopFile.c_str());
	GetTrieHops(pTrie, 0, 0, &fout, true);
	fout << flush;
	fout.close();

	ofstream fout1(oneHopFile.c_str());
	//if (pTrie->oldPort != EMPTYHOP)
		//fout1 << "0.0.0.0/0\t" << pTrie->oldPort << "\t" << endl;  
	GetTrieHops(pTrie, 0, 0, &fout1, false);
	fout1 << flush;
	fout1.close();
}


//get and output all the prefix & nexthops in Trie
void CFib::GetTrieHops(TrieNode* pTrie, unsigned int iVal, int iBitLen, ofstream* fout, bool ifTwoHop)
{
	unsigned int leftport, rightport;
	leftport = pTrie->newPort[0];
	rightport = pTrie->newPort[1];

	if ((pTrie->nodeType == UNITEDNODE || pTrie->nodeType == SPLITNODE) && (leftport != EMPTYHOP || rightport != EMPTYHOP))
	{
		char strVal[50];
		unsigned int Val;

		//output one prefix and two next hops
		if (ifTwoHop == true)
		{
			memset(strVal, 0, sizeof(strVal));
			sprintf_s(strVal, "%d.%d.%d.%d/%d\t%d\t%d\n", (iVal >> 24), (iVal << 8) >> 24, (iVal << 16) >> 24, (iVal << 24) >> 24, iBitLen, leftport, rightport);
			*fout << strVal;

		}
		//output two prefixes and corresponding next hops
		else
		{
			if (iBitLen == 32)
			{
				memset(strVal, 0, sizeof(strVal));
				sprintf_s(strVal, "%d.%d.%d.%d/%d\t%d\n", (iVal >> 24), (iVal << 8) >> 24, (iVal << 16) >> 24, (iVal << 24) >> 24, iBitLen, leftport);
				*fout << strVal;
			}
			else
			{
				if (leftport != EMPTYHOP && leftport != NOTEXIST)
				{
					memset(strVal, 0, sizeof(strVal));
					sprintf_s(strVal, "%d.%d.%d.%d/%d\t%d\n", (iVal >> 24), (iVal << 8) >> 24, (iVal << 16) >> 24, (iVal << 24) >> 24, iBitLen + 1, leftport);
					*fout << strVal;
				}
				if (rightport != EMPTYHOP && rightport != NOTEXIST)
				{
					Val = iVal + (1 << (31 - iBitLen));
					memset(strVal, 0, sizeof(strVal));
					sprintf_s(strVal, "%d.%d.%d.%d/%d\t%d\n", (Val >> 24), (Val << 8) >> 24, (Val << 16) >> 24, (Val << 24) >> 24, iBitLen + 1, rightport);
					*fout << strVal;
				}
			}
		}
	}

	iBitLen++;

	//try to handle the left sub-tree
	if (pTrie->lchild != NULL)
	{
		GetTrieHops(pTrie->lchild, iVal, iBitLen, fout, ifTwoHop);
	}
	//try to handle the right sub-tree
	if (pTrie->rchild != NULL)
	{
		iVal += 1 << (32 - iBitLen);
		GetTrieHops(pTrie->rchild, iVal, iBitLen, fout, ifTwoHop);
	}
}


void CFib::OutputCompressedTrieBin(TrieNode* pTrie, string twoHopFile, string oneHopFile){
	ofstream fout(twoHopFile.c_str());
	void OutputTwoHop2(TrieNode * pTrie, ofstream * fout);
	OutputTwoHop2(pTrie, &fout);
	fout << flush;
	fout.close();

	ofstream fout1(oneHopFile.c_str());
	if (pTrie->oldPort != EMPTYHOP)
		fout1 << "0\t" << pTrie->oldPort << "\t" << endl;
	void OutputOneHop2(TrieNode * pTrie, ofstream * fout);
	OutputOneHop2(pTrie, &fout1);
	fout1 << flush;
	fout1.close();

}

void OutputTwoHop2(TrieNode * pTrie, ofstream * fout)
{
	unsigned int leftport, rightport;
	leftport = pTrie->newPort[0];
	rightport = pTrie->newPort[1];

	if (leftport != NOTEXIST && (leftport != EMPTYHOP || rightport != EMPTYHOP))
	{
		stack<int> s;
		TrieNode *tmp = pTrie;
		while (tmp->parent != NULL){
			s.push(tmp->path);
			tmp = tmp->parent;
		}
		string out;
		while (!s.empty()){
			char foobar[PREFIXLEN];
			sprintf_s(foobar, "%d", s.top());
			out.append(foobar);
			s.pop();
		}
		*fout << out;
		*fout << '\t'
			<< leftport
			<< '\t'
			<< rightport
			<< endl;
	}

	if (pTrie->lchild != NULL){
		pTrie->lchild->path = 0;
		OutputTwoHop2(pTrie->lchild, fout);
	}
	if (pTrie->rchild != NULL){
		pTrie->rchild->path = 1;
		OutputTwoHop2(pTrie->rchild, fout);
	}
}

void OutputOneHop2(TrieNode * pTrie, ofstream * fout)
{
	unsigned int leftport, rightport;
	leftport = pTrie->newPort[0];
	rightport = pTrie->newPort[1];
	int size = 0;
	if (leftport != EMPTYHOP && leftport != NOTEXIST)
	{
		stack<int> s;
		TrieNode *tmp = pTrie;
		while (tmp->parent != NULL){
			s.push(tmp->path);
			tmp = tmp->parent;
		}
		size = s.size();
		string out;
		while (!s.empty()){
			char foobar[PREFIXLEN];
			sprintf_s(foobar, "%d", s.top());
			out.append(foobar);
			s.pop();
		}
		*fout << out;
		if (size < 32)
			*fout << "0";
		*fout << "\t" << leftport << endl;
	}
	if (size <32 && rightport != EMPTYHOP && rightport != NOTEXIST)
	{
		stack<int> s;
		TrieNode *tmp = pTrie;
		while (tmp->parent != NULL){
			s.push(tmp->path);
			tmp = tmp->parent;
		}
		int size = s.size();
		string out;
		while (!s.empty()){
			char foobar[PREFIXLEN];
			sprintf_s(foobar, "%d", s.top());
			out.append(foobar);
			s.pop();
		}
		*fout << out;
		if (size < 32)
			*fout << "1";
		*fout << "\t" << rightport << endl;
	}
	if (pTrie->lchild != NULL){
		pTrie->lchild->path = 0;
		OutputOneHop2(pTrie->lchild, fout);
	}
	if (pTrie->rchild != NULL){
		pTrie->rchild->path = 1;
		OutputOneHop2(pTrie->rchild, fout);
	}
}


//get and output all the prefix & nexthops in Trie
void CFib::OutPutNormalRib(TrieNode *pTrie, string oneHopFile)
{
	ofstream fout1(oneHopFile.c_str());
	GetOutPutNormalRib(pTrie, 0, 0, &fout1);
	fout1 << flush;
	fout1.close();
}

void CFib::GetOutPutNormalRib(TrieNode* pTrie, unsigned int iVal, int iBitLen, ofstream* fout)
{
	if (pTrie->oldPort != EMPTYHOP)
	{
		char strVal[50];
	
		memset(strVal, 0, sizeof(strVal));
		sprintf_s(strVal, "%d.%d.%d.%d/%d\t%d\n", (iVal >> 24), (iVal << 8) >> 24, (iVal << 16) >> 24, (iVal << 24) >> 24, iBitLen, pTrie->oldPort);
		*fout << strVal;
	}

	iBitLen++;

	//try to handle the left sub-tree
	if (pTrie->lchild != NULL)
	{
		GetOutPutNormalRib(pTrie->lchild, iVal, iBitLen, fout);
	}
	//try to handle the right sub-tree
	if (pTrie->rchild != NULL)
	{
		iVal += 1 << (32 - iBitLen);
		GetOutPutNormalRib(pTrie->rchild, iVal, iBitLen, fout);
	}
}

//convert IP data into binary data
void CFib::IpToBinary(string sIP, char saBin[PREFIXLEN]){
	int iStart = 0;				//the start point of IP
	int iEnd = 0;					//the end point of IP
	int iFieldIndex = 3;
	int iLen = sIP.length();		//the Length of IP
	unsigned long	lPrefix = 0;	//IP in integer format


	//turn IP into an integer
	for (int i = 0; i < iLen; i++){
		//extract the first three sub-parts
		if (sIP.substr(i, 1) == "."){
			iEnd = i;
			string strVal = sIP.substr(iStart, iEnd - iStart);
			lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex);
			iFieldIndex--;
			iStart = i + 1;
			i++;
		}
		if (iFieldIndex == 0){

			//extract the forth sub-part
			iEnd = iLen;
			string strVal = sIP.substr(iStart, iEnd - iStart);
			lPrefix += atol(strVal.c_str());
			iStart = i + 1;
			break;
		}
	}

	//turn into binary format stored in a array
	unsigned long	lVal = 0x80000000;
	int i;
	for (i = 0; i < 32; i++){
		if (lPrefix&lVal){
			saBin[i] = '1';
		}
		else{
			saBin[i] = '0';
		}
		lVal = lVal >> 1;
	}
	saBin[i] = '\0';
}

//add a node in Trie
void CFib::AddNode(unsigned long lPrefix, unsigned int iPrefixLen, unsigned int iNextHop)
{

	//get the root of trie
	TrieNode* pTrie = m_pTrie;
	//locate every prefix in the trie
	for (unsigned int i = 0; i < iPrefixLen; i++)
	{
		//turn right
		if (((lPrefix << i) & HIGHTBIT) == HIGHTBIT)
		{
			//creat new node
			if (pTrie->rchild == NULL)
			{
				TrieNode* pTChild = (struct TrieNode*)malloc(FIBLEN);
				if (!pTChild)
				{
					printf("Function AddNode malloc failed.");
					system("pause");
					exit(0);
				}
				//insert new node
				pTChild->parent = pTrie;
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				pTChild->oldPort = EMPTYHOP;
				pTChild->newPort[0] = EMPTYHOP;
				pTChild->newPort[1] = EMPTYHOP;
				pTChild->isAggregated = false;
				pTChild->nodeType = EMPTYNODE;

				pTrie->rchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->rchild;
		}
		//turn left
		else{
			//if left node is empty, creat a new node
			if (pTrie->lchild == NULL){
				TrieNode* pTChild = (struct TrieNode*)malloc(FIBLEN);

				if (!pTChild)
				{
					printf("Function AddNode malloc failed.");
					system("pause");
					exit(0);
				}

				//insert new node
				pTChild->parent = pTrie;
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				pTChild->oldPort = EMPTYHOP;
				pTChild->newPort[0] = EMPTYHOP;
				pTChild->newPort[1] = EMPTYHOP;
				pTChild->isAggregated = false;

				pTChild->nodeType = EMPTYNODE;
				pTrie->lchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->lchild;
		}
	}

	pTrie->oldPort = iNextHop;
	pTrie->newPort[0] = EMPTYHOP;
	pTrie->newPort[1] = EMPTYHOP;
	pTrie->isAggregated = false;
	pTrie->nodeType = EMPTYNODE;
}

//construct RIB tree from file in IP format,and return number of items in rib file
unsigned int CFib::BuildFibFromFile(string sFileName)
{
	unsigned int	iEntryCount = 0;	//the number of items from file
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of Prefix
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	ifstream fin(sFileName.c_str());
	while (!fin.eof())
	{
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix, 0, sizeof(sPrefix));

		fin >> sPrefix >> iNextHop;

		int iStart = 0;				//the start point of PREFIX
		int iEnd = 0;				//the start point of PREFIX
		int iFieldIndex = 3;
		int iLen = strlen(sPrefix);	//The length of PREFIX


		if (iLen > 0){
			iEntryCount++;
			for (int i = 0; i < iLen; i++){
				//get the first three sub-items
				if (sPrefix[i] == '.'){
					iEnd = i;
					string strVal(sPrefix + iStart, iEnd - iStart);
					lPrefix += atol(strVal.c_str()) << (8 * iFieldIndex); 
					iFieldIndex--;
					iStart = i + 1;
					i++;
				}
				if (sPrefix[i] == '/'){
					//get the prefix length
					iEnd = i;
					string strVal(sPrefix + iStart, iEnd - iStart);
					lPrefix += atol(strVal.c_str());
					iStart = i + 1;

					i++;
					strVal = string(sPrefix + iStart, iLen - 1);
					iPrefixLen = atoi(strVal.c_str());
				}
			}

			if (32 < iPrefixLen)
				//if (128<iPrefixLen)
			{
				printf("The prefix in FIB file is in wrong format!");
				system("pause");
				exit(0);
			}

			AddNode(lPrefix, iPrefixLen, iNextHop);

		}
	}
	fin.close();
	return iEntryCount;
}


unsigned int CFib::BuildFibFromBinFile(string sFileName)
{
	unsigned int	iEntryCount = 0;	//the number of items from file
	char			sPrefix[256];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of Prefix
	unsigned int	iNextHop;			//to store NEXTHOP in RIB file

	ifstream fin(sFileName.c_str());

	while (!fin.eof())
	{
		lPrefix = 0;
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix, '\0', sizeof(sPrefix));

		fin >> sPrefix >> iNextHop;

		if (sPrefix[0] == '*')
		{
			m_pTrie->oldPort = iNextHop;
			continue;
		}

		int iLen = strlen(sPrefix);	//The length of PREFIX
		TrieNode *current = m_pTrie;

		if (iLen == 0)
		{
			continue;
		}

		for (int i = 0; i < iLen; i++)
		{
			if (sPrefix[i] == '0')
			{
				if (current->lchild != NULL){
					current = current->lchild;
				}
				else{
					TrieNode * tmp;
					CreateNewNode(tmp);
					tmp->parent = current;
					current->lchild = tmp;
					current = tmp;
				}
			}
			else
			{

				if (current->rchild != NULL){
					current = current->rchild;
				}
				else{
					TrieNode * tmp;
					CreateNewNode(tmp);
					tmp->parent = current;
					current->rchild = tmp;
					current = tmp;
				}
			}
		}
		current->oldPort = iNextHop;
		iEntryCount++;
	}
	fin.close();
	return iEntryCount;
}


//sPrefix: binary ip address
void CFib::Normal_InsertNode(char *sPrefix, unsigned int iNextHop)
{
	//cout << "insert " << endl;
	int iLen = strlen(sPrefix);	//The length of PREFIX
	TrieNode *current = m_pTrie;

	for (int i = 0; i < iLen; i++)
	{
		if (sPrefix[i] == '0')
		{
			if (current->lchild != NULL){
				current = current->lchild;
			}
			else{
				TrieNode * tmp;
				CreateNewNode(tmp);
				tmp->parent = current;
				current->lchild = tmp;
				current = tmp;
			}
		}
		else
		{

			if (current->rchild != NULL){
				current = current->rchild;
			}
			else{
				TrieNode * tmp;
				CreateNewNode(tmp);
				tmp->parent = current;
				current->rchild = tmp;
				current = tmp;
			}
		}
	}//for

	current->oldPort = iNextHop;
}


int CFib::Normal_DeleteNode(char *sPrefix, unsigned int iNextHop)
{
	int iLen = strlen(sPrefix);	//The length of PREFIX
	TrieNode *current = m_pTrie;
	int path = 1;
	for (int i = 0; i < iLen; i++, path++)
	{
		if (sPrefix[i] == '0')
		{
			if (current->lchild != NULL){
				current = current->lchild;
			}
			else{
				return path;
			}
		}
		else
		{
			if (current->rchild != NULL){
				current = current->rchild;
			}
			else{
				return path;
			}
		}
	}//for

	path--;
	current->oldPort = EMPTYHOP;

	if (IsLeaf(current))
	{
		//delete the node		
		TrieNode *parent = current->parent;
		TrieNode *tmp = current;

		while (IsLeaf(tmp) && tmp->oldPort == EMPTYHOP)
		{
			if (parent->lchild && parent->lchild == tmp)
			{
				parent->lchild = NULL;
			}
			else if (parent->rchild && parent->rchild == tmp)
			{
				parent->rchild = NULL;
			}
			free(tmp);
			tmp = parent;
			parent = parent->parent;
		}
	}

	return path;
}


int CFib::US_InsertNode(char *sPrefix, unsigned int iNextHop)
{
	int iLen = strlen(sPrefix);	//The length of PREFIX
	TrieNode *current = m_pTrie;
	int path = iLen;

	for (int i = 0; i < iLen; i++)
	{
		if (sPrefix[i] == '0')
		{
			if (current->lchild != NULL){
				current = current->lchild;
			}
			else{
				TrieNode * tmp;
				CreateNewNode(tmp);
				tmp->parent = current;
				current->lchild = tmp;
				current = tmp;
			}
		}
		else
		{
			if (current->rchild != NULL){
				current = current->rchild;
			}
			else{
				TrieNode * tmp;
				CreateNewNode(tmp);
				tmp->parent = current;
				current->rchild = tmp;
				current = tmp;
			}
		}
	}//for

	current->oldPort = iNextHop;

	if (current->nodeType == EMPTYNODE)  //empty node or new
	{
		TrieNode *parent = current->parent;
		if (parent->nodeType == UNITEDNODE)
		{
			path--;// just update her parent 

			if (parent->lchild == current) //current node is lchild
				parent->newPort[0] = current->oldPort;
			else
				parent->newPort[1] = current->oldPort;
		
			current->isAggregated = true;
			current->nodeType = PARTNODE;
		}
		else if (parent->nodeType == SPLITNODE)
		{
			path--;// just update her parent 

			if (parent->lchild == current)
				parent->newPort[0] = current->oldPort;
			else
				parent->newPort[1] = current->oldPort;
			
			parent->isAggregated = true;
			parent->nodeType = UNITEDNODE;
			current->isAggregated = true;
			current->nodeType = PARTNODE;
		}
		else if (parent->nodeType == EMPTYNODE)
		{
			current->newPort[0] = current->newPort[1] = current->oldPort;
			current->nodeType = SPLITNODE;
			if (parent->lchild && parent->rchild && parent->lchild->nodeType == SPLITNODE && parent->rchild->nodeType == SPLITNODE)
			{
				path--; // just update her parent 
				parent->newPort[0] = parent->lchild->oldPort;
				parent->newPort[1] = parent->rchild->oldPort;
				parent->isAggregated = true;
				parent->nodeType = UNITEDNODE;

				parent->lchild->newPort[0] = EMPTYHOP;
				parent->lchild->newPort[1] = EMPTYHOP;
				parent->rchild->newPort[0] = EMPTYHOP;
				parent->rchild->newPort[1] = EMPTYHOP;

				parent->lchild->isAggregated = true;//add
				parent->lchild->nodeType = PARTNODE;//add

				parent->rchild->isAggregated = true;
				parent->rchild->nodeType = PARTNODE;
			}
			else
			{
				if (current->lchild && current->lchild->nodeType == SPLITNODE)
				{
					path--;// just update her parent 
					current->newPort[0] = current->lchild->oldPort;
					current->isAggregated = true;
					current->nodeType = UNITEDNODE;
					current->lchild->isAggregated = true;
					current->lchild->newPort[0] = current->lchild->newPort[1] = EMPTYHOP;
					current->lchild->nodeType = PARTNODE;
				}
				else if (current->rchild && current->rchild->nodeType == SPLITNODE)
				{
					path--; // just update her parent 
					current->newPort[1] = current->rchild->oldPort;
					current->isAggregated = true;
					current->nodeType = UNITEDNODE;
					current->rchild->isAggregated = true;
					current->rchild->newPort[0] = current->rchild->newPort[1] = EMPTYHOP;
					current->rchild->nodeType = PARTNODE;
				}
			}
		}// parent is empty node
		else if (parent->nodeType == PARTNODE)
		{
			current->newPort[0] = current->newPort[1] = current->oldPort;
			current->nodeType = SPLITNODE;
		}
	}//if 
	else if (current->nodeType == SPLITNODE)
	{
		current->newPort[0] = current->newPort[1] = current->oldPort;
	}
	else if (current->nodeType == PARTNODE)
	{
		path--;// just update her parent 
		TrieNode *parent = current->parent;

		if (parent->lchild == current)
			parent->newPort[0] = current->oldPort;
		else if (parent->rchild == current)
			parent->newPort[1] = current->oldPort;
	}
	else if (current->nodeType == UNITEDNODE)
	{
		if (current->lchild && current->rchild && current->lchild->nodeType == PARTNODE && current->rchild->nodeType == PARTNODE)
		{
		}
		else if (current->lchild && current->lchild->nodeType == PARTNODE)
		{
			current->newPort[1] = current->oldPort;
		}
		else if (current->rchild && current->rchild->nodeType == PARTNODE)
		{
			current->newPort[0] = current->oldPort;
		}
	}

	return path;
}


int CFib::US_DeleteNode(char *sPrefix, unsigned int iNextHop)
{
	int iLen = strlen(sPrefix);	//The length of PREFIX
	TrieNode *current = m_pTrie;

	int path = 1;
	for (int i = 0; i < iLen; i++)
	{
		if (sPrefix[i] == '0')
		{
			if (current->lchild != NULL)
				current = current->lchild;
			else
				return path;
		}
		else
		{
			if (current->rchild != NULL)
				current = current->rchild;
			else
				return path;
		}
		path++;
	}//for	
	path--;

	current->oldPort = EMPTYHOP;
	if (current->nodeType == UNITEDNODE)  
	{
		if (current->lchild && current->rchild && current->lchild->nodeType == PARTNODE && current->rchild->nodeType == PARTNODE)
		{
			current->oldPort = EMPTYHOP;
		}
		else
		{
			current->oldPort = EMPTYHOP;
			current->newPort[0] = current->newPort[1] = EMPTYHOP;
			current->nodeType = EMPTYNODE;
			TrieNode *child;

			if (current->lchild && current->lchild->nodeType == PARTNODE)
				child = current->lchild;
			else
				child = current->rchild;

			child->newPort[0] = child->newPort[1] = child->oldPort;

			char flag = 0;
			if (child->lchild && child->lchild->nodeType == SPLITNODE)
			{
				child->newPort[0] = child->lchild->oldPort;
				child->nodeType = UNITEDNODE;
				child->lchild->isAggregated = true;
				child->lchild->newPort[0] = child->lchild->newPort[1] = EMPTYHOP;
				child->lchild->nodeType = PARTNODE;
				flag = 1;
			}
			if (child->rchild && child->rchild->nodeType == SPLITNODE)
			{
				child->newPort[1] = child->rchild->oldPort;
				child->nodeType = UNITEDNODE;
				child->rchild->isAggregated = true;
				child->rchild->newPort[0] = child->rchild->newPort[1] = EMPTYHOP;
				child->rchild->nodeType = PARTNODE;
				flag = 2;
			}

			if (flag == 0)
			{
				child->nodeType = SPLITNODE;
			}
		}//else
	}
	else if (current->nodeType == PARTNODE) 
	{
		path--;// just update her parent 

		TrieNode *parent = current->parent;
		TrieNode *brother = NULL;
		bool hasBrother = 0;

		if (parent->lchild && parent->rchild)
		{
			hasBrother = 1;
			if (parent->lchild == current)
				brother = parent->rchild;
			else
				brother = parent->lchild;
		}

		if (hasBrother == 1 && brother->nodeType == PARTNODE) // brother is parti.
		{
			if (parent->oldPort != EMPTYHOP && parent->lchild == current)  // father has next hop
			{
				parent->newPort[0] = parent->oldPort;  //combine
			}
			else if (parent->oldPort != EMPTYHOP && parent->rchild == current)
			{
				parent->newPort[1] = parent->oldPort;
			}
			else if (parent->oldPort == EMPTYHOP) // parent ->oldport  is empty
			{
				parent->newPort[0] = parent->newPort[1] = EMPTYHOP;
				parent->nodeType = EMPTYNODE;

				brother->newPort[0] = brother->newPort[1] = brother->oldPort;
				brother->nodeType = SPLITNODE;

				if (brother->lchild && brother->lchild->nodeType == SPLITNODE)
				{
					brother->newPort[0] = brother->lchild->oldPort;
					brother->lchild->isAggregated = true;
					brother->lchild->nodeType = PARTNODE;
					brother->nodeType = UNITEDNODE;
					brother->isAggregated = true;
				}
				if (brother->rchild && brother->rchild->nodeType == SPLITNODE)
				{
					brother->newPort[1] = brother->rchild->oldPort;
					brother->rchild->isAggregated = true;
					brother->rchild->nodeType = PARTNODE;
					brother->nodeType = UNITEDNODE;
					brother->isAggregated = true;
				}
			}//else
		}//if brother is parti.
		else // brother is not p or has no brother
		{
			TrieNode *Grandpa = parent->parent;
			if (Grandpa->nodeType == EMPTYNODE)
			{
				if (Grandpa->lchild == parent)
				{
					if (Grandpa->rchild && Grandpa->rchild->nodeType == SPLITNODE)
					{
						Grandpa->newPort[0] = parent->oldPort;
						Grandpa->newPort[1] = Grandpa->rchild->oldPort;
						Grandpa->nodeType = UNITEDNODE;
						Grandpa->isAggregated = true;

						parent->nodeType = PARTNODE;
						parent->newPort[0] = parent->newPort[1] = EMPTYHOP;

						Grandpa->rchild->nodeType = PARTNODE;
						Grandpa->rchild->newPort[0] = Grandpa->rchild->newPort[1] = EMPTYHOP;
						Grandpa->rchild->isAggregated = true;
					}
					else
					{
						parent->newPort[0] = parent->newPort[1] = parent->oldPort;
						parent->nodeType = SPLITNODE;
					}
				}
				else if (Grandpa->rchild == parent)
				{
					if (Grandpa->lchild && Grandpa->lchild->nodeType == SPLITNODE)
					{
						Grandpa->newPort[0] = Grandpa->lchild->oldPort;
						Grandpa->newPort[1] = parent->oldPort;
						Grandpa->nodeType = UNITEDNODE;
						Grandpa->isAggregated = true;

						parent->nodeType = PARTNODE;
						parent->newPort[0] = parent->newPort[1] = EMPTYHOP;

						Grandpa->lchild->nodeType = PARTNODE;
						Grandpa->lchild->isAggregated = true;
						Grandpa->lchild->newPort[0] = Grandpa->lchild->newPort[1] = EMPTYHOP;
					}
					else
					{
						parent->newPort[0] = parent->newPort[1] = parent->oldPort;
						parent->nodeType = SPLITNODE;
					}
				}
			}// Grandpa is empty
			else if (Grandpa->nodeType == UNITEDNODE || Grandpa->nodeType == SPLITNODE)
			{
				if (Grandpa->lchild == parent)
					Grandpa->newPort[0] = parent->oldPort;
				else
					Grandpa->newPort[1] = parent->oldPort;

				Grandpa->nodeType = UNITEDNODE;  // change the case TYPESPLITNODE
				parent->nodeType = PARTNODE;
				parent->newPort[0] = parent->newPort[1] = EMPTYHOP;
			}
			else  // Grandpa->nodeType == TYPEPARTNODE
			{
				parent->newPort[0] = parent->newPort[1] = parent->oldPort;
				parent->nodeType = SPLITNODE;
			}
		}// brother is not p 

		current->oldPort = EMPTYHOP; 
		current->nodeType = EMPTYNODE;
		current->newPort[0] = current->newPort[1] = EMPTYHOP;

		if (current->lchild && current->rchild && current->lchild->nodeType == SPLITNODE && current->rchild->nodeType == SPLITNODE)/// lchild and rchild both split nodes
		{
			current->newPort[0] = current->lchild->oldPort;
			current->newPort[1] = current->rchild->oldPort;
			current->isAggregated = true;
			current->nodeType = UNITEDNODE;
			current->lchild->isAggregated = true;
			current->lchild->nodeType = PARTNODE;
			current->lchild->newPort[0] = current->lchild->newPort[1] = EMPTYHOP;
			current->rchild->isAggregated = true;
			current->rchild->nodeType = PARTNODE;
			current->rchild->newPort[0] = current->rchild->newPort[1] = EMPTYHOP;
		}
		else if (IsLeaf(current))
		{
			//delete the node		
			TrieNode *parent = current->parent;
			TrieNode *tmp = current;

			while (IsLeaf(tmp) && tmp->nodeType == EMPTYNODE)
			{
				if (parent->lchild && parent->lchild == tmp)
					parent->lchild = NULL;
				else if (parent->rchild && parent->rchild == tmp)
					parent->rchild = NULL;
			
				free(tmp);
				tmp = parent;
				parent = parent->parent;
			}
		}
		else
		{
			current->isAggregated = false;
			current->nodeType = EMPTYNODE;
			current->newPort[0] = current->newPort[1] = EMPTYHOP;
		}
	}//else if  n is part
	else if (current->nodeType == SPLITNODE)
	{
		current->nodeType = EMPTYNODE;

		if (IsLeaf(current))
		{
			//delete the node
			TrieNode *parent = current->parent;
			TrieNode *tmp = current;
			while (IsLeaf(tmp) && tmp->nodeType == EMPTYNODE)
			{
				if (parent->lchild && parent->lchild == tmp)
					parent->lchild = NULL;
				else if (parent->rchild && parent->rchild == tmp)
					parent->rchild = NULL;
				
				free(tmp);
				tmp = parent;
				parent = parent->parent;
			}
		}
		else
		{
			current->isAggregated = false;
			current->oldPort = EMPTYHOP;
			current->newPort[0] = current->newPort[1] = EMPTYHOP;
		}
	}// n split node

	return path;
}

/*
convert the ip of update file in binary
sIpFile: the update file name (in the file,IP is like: 10.0.0.1)
sBinFile: the result file(in the file,IP is in binary )
*/
void CFib::ConvertUpdateFileIpToBin(string sIpFile, string sBinFile)
{
	char			sBinPrefix[PREFIXLEN];		//PREFIX in binary format
	string			strIpPrefix;				//PREFIX in IP format
	unsigned int	iPrefixLen;					//the length of PREFIX
	unsigned int	iNextHop;					//to store NEXTHOP in RIB file
	unsigned int	iEntryCount = 0;			//the number of items that is transformed successfully

	char			sPrefix[PREFIXLEN];			//store the prefix from rib file
	string sTime, sNumber;
	char updateType;
	cout << " Convert UpdateFile Ip to Bin " << sIpFile << " " << sBinFile << endl;
	//open the file and prepare to store the routing information in binary format
	ofstream fout(sBinFile.c_str());

	//open the RIB file in IP format
	ifstream fin(sIpFile.c_str());
	while (!fin.eof())
	{
		iPrefixLen = 0;
		iNextHop = EMPTYHOP;

		memset(sPrefix, 0, sizeof(sPrefix));
		fin >> sTime >> sNumber >> updateType >> sPrefix >> iNextHop;

		int iLen = strlen(sPrefix);	

		if (iLen > 0)
		{
			iEntryCount++;
			for (int i = 0; i < iLen; i++)
			{
				if (sPrefix[i] == '/')
				{
					//extract the forth sub-part
					string strVal(sPrefix, i);
					strIpPrefix = strVal;

					//extract the length of prefix
					strVal = string(sPrefix + i + 1, iLen - 1);
					iPrefixLen = atoi(strVal.c_str());
					break;
				}
			}

			memset(sBinPrefix, 0, sizeof(sBinPrefix));
			//convert IP data into binary data
			IpToBinary(strIpPrefix, sBinPrefix);

			//to handle the root whose the length of prefix is 0
			if (iPrefixLen > 0)
			{
				strIpPrefix = string(sBinPrefix, iPrefixLen);
			}
			else
			{
				strIpPrefix = "*";
			}

			fout << sTime << "\t" << sNumber << "\t" << updateType << "\t" << strIpPrefix << "\t" << iNextHop << endl;
		}
	}

	fin.close();
	fout << flush;
	fout.close();
}


void CFib::UpdateFromFile2(string sIpFile, char algorithmType, ofstream* fout)
{
	unsigned int	iNextHop;				//to store NEXTHOP in RIB file
	unsigned int	iEntryCount = 0;		//the number of items that is transformed successfully
	char			sPrefix[PREFIXLEN];		//store the prefix from rib file
	string sDay, sTime;
	char updateType;

#ifdef DEBUG
	int total = 0;
#endif

	std::cout << "update from file...." << endl;

	string str_pre_min = "-1";
	string str_now_min = "";
	int total_min = 0;	
	float update_rib_num = 1;
	int mem_access = 0;
	int arr_mem_acc[32];
	memset(arr_mem_acc, 0, sizeof(arr_mem_acc));

	int avg_rib_num = 0;

	//open the RIB file in IP format
	ifstream fin(sIpFile.c_str());

	LARGE_INTEGER frequence, begin, end;
	if (!QueryPerformanceFrequency(&frequence))return;
	long long updateTimeOfOneMin = 0, total_updateTimeOfOneMin=0;
	
	while (!fin.eof())
	{
#ifdef DEBUG
		total++;
		if (total % 100000 == 0)
		{
			cout << "100,000" << endl;
		}
#endif

		iNextHop = EMPTYHOP;
		memset(sPrefix, 0, sizeof(sPrefix));
		fin >> sDay >> sTime >> updateType >> sPrefix >> iNextHop;

#ifdef DEBUG	
		cout << "update " << sDay << "\t" << sTime << "\t" << updateType << "\t" << sPrefix << "\t" << iNextHop << endl;
#endif

		str_now_min = "";
		str_now_min = sTime.substr(0, 4);
		if (str_now_min != str_pre_min)
		{
			GetNodeCounts3(algorithmType);	

			*fout << total_min++ << "\t";

			// size 
			if (algorithmType == BASIC)
				*fout << allNodeCount;
			else if (algorithmType == US)
				*fout << allNodeCount;

			*fout << "\t" << total_updateTimeOfOneMin << "\t" << mem_access / update_rib_num << "\t" << avg_rib_num << "\t" << updateTimeOfOneMin << "\t";
			if (updateTimeOfOneMin !=0)
				*fout << float(avg_rib_num) / updateTimeOfOneMin << "\t";
			else
				*fout << "0" << "\t";

			*fout << endl;
			str_pre_min = str_now_min;

			mem_access = 0;
			update_rib_num = 1;
			avg_rib_num = 0;
			updateTimeOfOneMin = 0;
		}

		if (iNextHop == EMPTYHOP) // skip the blank row.
		{
			continue;
		}

		update_rib_num += 1;
		avg_rib_num += 1;
		int flag;
		if (updateType == 'A')
		{
			flag = strlen(sPrefix);
			if (algorithmType == BASIC)
			{
				QueryPerformanceCounter(&begin);
				Normal_InsertNode(sPrefix, iNextHop);
				QueryPerformanceCounter(&end);
			}
			else if (algorithmType == US)
			{
				QueryPerformanceCounter(&begin);
				flag = US_InsertNode(sPrefix, iNextHop);
				QueryPerformanceCounter(&end);
			}
		}
		else if (updateType == 'W')
		{
			flag = 1;
			if (algorithmType == BASIC)
			{
				QueryPerformanceCounter(&begin);
				flag = Normal_DeleteNode(sPrefix, iNextHop);
				QueryPerformanceCounter(&end);
			}
			else if (algorithmType == US)
			{
				QueryPerformanceCounter(&begin);
				flag = US_DeleteNode(sPrefix, iNextHop);
				QueryPerformanceCounter(&end);
			}			
		}//else if

		updateTimeOfOneMin += 1000000 * (end.QuadPart - begin.QuadPart) / frequence.QuadPart;
		total_updateTimeOfOneMin += 1000000 * (end.QuadPart - begin.QuadPart) / frequence.QuadPart;
		mem_access += flag;
		arr_mem_acc[flag - 1]++;
	}//while

	fin.close();
	*fout << "TOTAL static" << endl;
	*fout << "mem_acc_times \t" << "number" << endl;
	for (int i = 0; i < 32; i++)
	{
		*fout << i + 1 << "\t" << arr_mem_acc[i] << endl;
	}
}

void CFib::UpdateFromFile(string sIpFile, char algorithmType)
{
	unsigned int	iNextHop;				//to store NEXTHOP in RIB file
	unsigned int	iEntryCount = 0;		//the number of items that is transformed successfully
	char			sPrefix[PREFIXLEN];		//store the prefix from rib file
	string sTime, sNumber;
	char updateType;

	//open the RIB file in IP format
	ifstream fin(sIpFile.c_str());

#ifdef DEBUG
	int total = 0;
#endif
	std::cout << "update from file...." << endl;

	while (!fin.eof())
	{
#ifdef DEBUG
		total++;
		if (total % 100000 == 0)
		{
			cout << "100,000" << endl;
		}
#endif


		iNextHop = EMPTYHOP;

		memset(sPrefix, 0, sizeof(sPrefix));
		fin >> sTime >> sNumber >> updateType >> sPrefix >> iNextHop;

#ifdef DEBUG	
		cout << "update " << sTime << "\t" << sNumber << "\t" << updateType << "\t" << sPrefix << "\t" << iNextHop << endl;
#endif

		if (iNextHop == EMPTYHOP) // skip the blank row.
		{
			continue;
		}
		if (updateType == 'A')
		{
			if (algorithmType == BASIC)
			{
				Normal_InsertNode(sPrefix, iNextHop);
			}
			else if (algorithmType == US)
			{
				US_InsertNode(sPrefix, iNextHop);
			}
		}
		else if (updateType == 'W')
		{
			if (algorithmType == BASIC)
			{
				Normal_DeleteNode(sPrefix, iNextHop);
			}
			else if (algorithmType == US)
			{
				US_DeleteNode(sPrefix, iNextHop);
			}
		}
	}
	fin.close();
}


unsigned int * CFib::TrafficRead(char *traffic_file)
{
	unsigned int *traffic = new unsigned int[TRACE_READ];
	int return_value = -1;
	unsigned int traceNum = 0;

	//first read the trace...
	ifstream fin(traffic_file);
	if (!fin)return 0;
	fin >> traceNum;

	int TraceLine = 0;
	unsigned int IPtmp = 0;
	while (!fin.eof() && TraceLine<TRACE_READ)
	{
		fin >> IPtmp;
		traffic[TraceLine] = IPtmp;
		TraceLine++;
	}
	fin.close();
	printf("trace read complete...\n");

	if (TraceLine<TRACE_READ)
	{
		printf("not enough\n", TraceLine);
	}

	return traffic;
}


int CFib::lookupOneIP_binary(unsigned int IP)
{
	TrieNode *pNode = m_pTrie;
	register int nexthop = 0;

	register char i = 0;
	while (pNode)
	{
		if (pNode->oldPort>0)nexthop = pNode->oldPort;

		if (IP << i >> 31) 
		{
			pNode = pNode->rchild;
		}
		else
		{
			pNode = pNode->lchild;
		}
		i++;
	}

	return nexthop;
}


int CFib::lookupOneIP_compress(unsigned int IP)
{
	TrieNode *pNode = m_pTrie;
	register int nexthop = 0;

	register char i = 0;
	while (pNode)
	{
		if (IP << i >> 31) 
		{
			if (pNode->newPort[1]>0)nexthop = pNode->newPort[1];
				pNode = pNode->rchild;
		}
		else
		{
			if (pNode->newPort[0]>0)nexthop = pNode->newPort[0];
				pNode = pNode->lchild;
		}
		i++;
	}

	return nexthop;
}
