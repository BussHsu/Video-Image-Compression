#pragma once
#include <stdio.h>
#include <string>
#include <map>
#include <vector>

//implement LZW encoding input range 0-255(unsinged char) to 
class LZWTrie
{
	friend class LZW;
	const unsigned char MAX_BITNUM = 16;

	unsigned short HALT_SIGNAL = 510;
	unsigned short EOF_SIGNAL = 511;

	struct LZWTrieNode 
	{
		unsigned short codeword;
		LZWTrieNode* children[256];
		bool isLeaf = false;
		LZWTrieNode(unsigned short codewordIn, bool isLeaf = true);
		~LZWTrieNode();
	} root;	//define the Node struct and declare a root

	//ctor, dtor
	LZWTrie();
	~LZWTrie();
	

	unsigned short lastEncoding = 255;
	LZWTrieNode* currNode;
	int lastChar = -1;
	//start with 9 bits encoding
	unsigned char bitNum = 9;
	int dictionarySize = pow(2,8)-2;

	unsigned short GetNextCodeword();
	void AddChild(LZWTrieNode* node,unsigned char i);
	
public:
	//match and return the codeword of the query, return 510 if encode new word, return 511 if expand bit size and encode new word
	unsigned short MatchAndEncode(unsigned char input);
	unsigned short GetCurrWord() { return currNode->codeword; }
};

class LZW
{

	LZWTrie m_Dictionary;
public:
	LZW();
	~LZW();

	int encode(unsigned char* in, unsigned int size, const std::string filePath);

};

class LZWDecoder 
{
	
	unsigned char bitNum = 9;
	int dictionarySize = pow(2, 8) - 2; //(256-509)
	std::map<unsigned short, std::vector<unsigned char>> m_decodeMap;

	unsigned short HALT_SIGNAL = 510;
	unsigned short EOF_SIGNAL = 511;
public:
	~LZWDecoder();
	void decode(std::string codeFilePath,std::string outFilePath);

private:
	unsigned short AlignAndExtract(unsigned short in, unsigned short& prev, unsigned char& numBitsLeft);
};