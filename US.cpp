#include "US.h"
#include <iostream>
#include <iomanip>

#define	USFIBLEN			sizeof(struct USTrieNode)		//size of each Trie Node
#define EMPTYHOP			0							//Trie node that does not have a next hop
#define NOTEXIST            -1							//Trie node that does not exist after US
#define HIGHTBIT			2147483648					//Binary: 10000000000000000000000000000000
using namespace std;

USFib::USFib(void)
{
	m_pTrie = (struct USTrieNode*)malloc(USFIBLEN);
	m_pTrie->lchild = NULL;
	m_pTrie->rchild = NULL;
	m_pTrie->leftport = EMPTYHOP;
	m_pTrie->rightport = EMPTYHOP;
}

USFib::~USFib(void)
{
}

/*
*PURPOSE: construct RIB tree from file in IP format
*RETURN VALUES: number of items in rib file
*/
unsigned int USFib::BuildFibFromFile(char * sFileName)
{
	unsigned int	iEntryCount = 0;	//the number of items from file
	char			sPrefix[20];		//prefix from rib file
	unsigned long	lPrefix;			//the value of Prefix
	unsigned int	iPrefixLen;			//the length of Prefix
	unsigned int	leftport;			//to store NEXTHOP in RIB file
	unsigned int	rightport;			//to store NEXTHOP in RIB file

	ifstream fin(sFileName);
	while (!fin.eof())
	{
		lPrefix = 0;
		iPrefixLen = 0;
		leftport =rightport = EMPTYHOP;

		memset(sPrefix, 0, sizeof(sPrefix));

		fin >> sPrefix >> leftport >> rightport;
		int iStart = 0;				//the start point of PREFIX
		int iEnd = 0;				//the start point of PREFIX
		int iFieldIndex = 3;
		int iLen = strlen(sPrefix);	//The length of PREFIX

		if (iLen > 0)
		{
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
			{
				printf("The prefix in FIB file is in wrong format!");
				system("pause");
				exit(0);
			}

			AddNode(lPrefix, iPrefixLen, leftport, rightport);

		}
		else
		{
			break;
		}
	}
	fin.close();
	return iEntryCount;
}


//add a node in Trie
void USFib::AddNode(unsigned long lPrefix, unsigned int iPrefixLen, unsigned int leftport, unsigned int rightport)
{

	//get the root of trie
	USTrieNode* pTrie = m_pTrie;

	//locate every prefix in the trie
	for (unsigned int i = 0; i < iPrefixLen; i++)
	{
		if (!pTrie)
		{
			printf("location II failed!\n");
		}
		//turn right
		if (((lPrefix << i) & HIGHTBIT) == HIGHTBIT)
		{
			//creat new node
			if (pTrie->rchild == NULL)
			{
				USTrieNode* pTChild = (struct USTrieNode*)malloc(USFIBLEN);
				if (!pTChild)
				{
					printf("Function AddNode malloc failed.");
					system("pause");
					exit(0);
				}
				//insert new node
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				pTChild->leftport = EMPTYHOP;
				pTChild->rightport = EMPTYHOP;

				pTrie->rchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->rchild;
		}
		//turn left
		else{
			//if left node is empty, creat a new node
			if (pTrie->lchild == NULL){
				USTrieNode* pTChild = (struct USTrieNode*)malloc(USFIBLEN);

				if (!pTChild)
				{
					printf("Function AddNode malloc failed.");
					system("pause");
					exit(0);
				}

				//insert new node
				pTChild->lchild = NULL;
				pTChild->rchild = NULL;
				pTChild->leftport = EMPTYHOP;
				pTChild->rightport = EMPTYHOP;

				pTrie->lchild = pTChild;
			}
			//change the pointer
			pTrie = pTrie->lchild;
		}
	}

	pTrie->leftport = leftport;
	pTrie->rightport = rightport;
}


unsigned int USFib::lookupOneIP_compress(unsigned int IP)
{
	USTrieNode *pNode = m_pTrie;
	unsigned int nexthop=0;

	register int i=0;
	while (pNode)
	{
		if(IP<<i>>31) 
		{
			if(pNode->rightport>0)nexthop=pNode->rightport;
			pNode=pNode->rchild;
		}
		else
		{
			if(pNode->leftport>0)nexthop=pNode->leftport;
			pNode=pNode->lchild;
		}
		i++;
	}

	return nexthop;
}