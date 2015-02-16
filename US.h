#ifndef USFIB_H_
#define USFIB_H_
#include	<string>
#include	<fstream>

struct USTrieNode
{
	USTrieNode  *	lchild;					//point to the left child(0)
	USTrieNode  *	rchild;					//point to the right child(1)
	unsigned int leftport;
	unsigned int rightport;
};

class USFib
{
public:
	USTrieNode * m_pTrie;			//root TrieNode

	USFib(void);
	~USFib(void);

	unsigned int lookupOneIP_compress(unsigned int IP);
	unsigned int BuildFibFromFile(char * sFileName);
	void USFib::AddNode(unsigned long lPrefix, unsigned int iPrefixLen, unsigned int leftport, unsigned int rightport);
};

#endif /* US_H_ */