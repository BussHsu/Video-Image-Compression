#include "LZW.h"
#include <math.h>
#include <fstream>
#include <iostream>
LZW::LZW()
{

}
LZW::~LZW()
{
}

int LZW::encode(unsigned char* in, unsigned int size, const std::string filePath)
{
	std::ofstream myfile(filePath, std::ios::out| std::ios::binary);
	if (myfile.is_open())
	{
		//output byte
		unsigned char bitsLeft = 0;
		//num of bits left from previus output
		unsigned char numBitsLeft = 0;
		unsigned char bitNum;
		for (unsigned int i = 0; i <= size; i++)
		{
			bitNum = m_Dictionary.bitNum;
			unsigned short temp;
			if (i == size)
				temp = m_Dictionary.GetCurrWord(); 
			else
			{
				temp = m_Dictionary.MatchAndEncode(*(in + i));
			}
				
			
			if (temp == m_Dictionary.HALT_SIGNAL)
				continue;
			//debug
			std::cout << temp << " ";
			unsigned char totalBits = bitNum + numBitsLeft;
			
			while (totalBits>7)
			{
				//output: current codeword left shift numBitsLeft%8 & bitsLeft
				unsigned char out = (unsigned char)((temp << numBitsLeft)|bitsLeft);
				myfile<<out;
				temp = temp >> (8-numBitsLeft);

				totalBits -= 8;
				if (totalBits > 7)
				{
					numBitsLeft = 0;
				}
				else
				{
					numBitsLeft = totalBits;
					bitsLeft = temp;
				}
			}
		}
		//flush the remaining to output
		unsigned short eof = m_Dictionary.EOF_SIGNAL;
		short totalBits = numBitsLeft + 9;
		while (totalBits > 0)
		{
			//output: current codeword left shift numBitsLeft%8 & bitsLeft
			myfile << (unsigned char)((eof << numBitsLeft)|bitsLeft);
			bitsLeft = eof >> (8-numBitsLeft);
			totalBits -= 8;
		}

		myfile.close();
	}
	else
		return -1;
	return 0;
}

LZWTrie::LZWTrieNode::LZWTrieNode(unsigned short codewordIn,bool isLeaf) :
	children(),
	codeword(codewordIn),
	isLeaf(isLeaf)
{
};
LZWTrie::LZWTrieNode::~LZWTrieNode()
{
	if (!isLeaf)
		printf("warning: deleting non leaf node");
	for (LZWTrieNode* node : children)
	{
		if (node) delete node;
	}
}

//Should never match root, start with unused codeword
LZWTrie::LZWTrie():
	root(510),
	currNode(&root),
	lastChar(-1)
{
	for (unsigned short i = 0; i < 256; i++)
	{
		root.children[i] = new LZWTrieNode(i);
		root.isLeaf = false;
	}
}

LZWTrie::~LZWTrie()
{

}

unsigned short LZWTrie::GetNextCodeword()
{
	lastEncoding++;
	if (lastEncoding == 510)
	{
		lastEncoding += 2;
		return lastEncoding;
	}
	//reserve all the codeword with prifix 111111111 as EOF
	if (lastEncoding >> (bitNum - 9) == 511)
	{
		lastEncoding += pow(2, bitNum - 9);
	}
	return lastEncoding;
}

void LZWTrie::AddChild(LZWTrieNode * node, unsigned char i)
{
	node->children[i] = new LZWTrieNode(GetNextCodeword());
	node->isLeaf = false;
	dictionarySize--;
	//if dictionarySize <0 => allow more bits to use
	if (dictionarySize < 0)
	{
		if (bitNum >= MAX_BITNUM)
		{
			//flush the least recent use
		}
		else
		{
			//expand number of encoding bits
			dictionarySize += pow(2,bitNum++);
			//TODO: output the RESIZE_SIGNAL
			EOF_SIGNAL *= 2;  //111111111 -> 11111111110
		}
	}
}

unsigned short LZWTrie::MatchAndEncode(unsigned char input)
{
	//output only after modifying the trie
	//if found a match => halt signal
	if (currNode->children[input])
	{
		currNode = currNode->children[input];
			return HALT_SIGNAL;		
	}
	
	//no match => create new TrieNode, reset the currNode ptr to root, save the last character, output the previous node's codeword
	AddChild(currNode,input);
	unsigned short out = currNode->codeword;
	//save the last character
	currNode = root.children[input];
	lastChar = currNode->codeword;
	return out;
}

LZWDecoder::~LZWDecoder()
{
	m_decodeMap.clear();
}

void LZWDecoder::decode(std::string codeFilePath, std::string outFilePath)
{
	unsigned char* oData = NULL;
	size_t size = 0;
	std::ifstream ifs(codeFilePath, std::ios::in|std::ios::binary);
	if (ifs.is_open())
	{
		ifs.seekg(0, std::ios::end);	// ptr to end
		size = ifs.tellg();				// get the length of the file
		ifs.seekg(0, std::ios::beg);	// reset ptr

		oData = new unsigned char[size]; 
		ifs.read((char*)oData, size);
	}
	ifs.close();

	//if (oData)
	//{
	//	printf("read file error, LZW.cpp @ LZWDECODER::decode");
	//	return;
	//}
	
	unsigned char numBitsLeft = 0;
	unsigned short bitsLeft = 0;

	for (int i = 0; i < size; i++)
	{
		unsigned char temp = *(oData + i);
		unsigned short codeword = AlignAndExtract(temp,bitsLeft,numBitsLeft);
		if (codeword == HALT_SIGNAL)
			continue;

		std::cout << codeword << " ";
		//decode and adjust dictionary size
	}

	delete oData;
}

unsigned short LZWDecoder::AlignAndExtract(unsigned short in, unsigned short& prev, unsigned char& numBitsLeft)
{
	if (8 + numBitsLeft >= bitNum)
	{
		unsigned short out = in;
		//debug
		out = ((out << (16 - bitNum + numBitsLeft)) >> (16 - bitNum)) | prev;
		prev = in >> (bitNum - numBitsLeft);
		numBitsLeft -= (bitNum - 8);
		return out;
	}
	
	prev |= (in << numBitsLeft);
	numBitsLeft += 8;

	return HALT_SIGNAL;
}
