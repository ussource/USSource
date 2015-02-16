#include "Fib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include "US.h"

#define CYCLEs 100

using namespace std;


#define  FILELEN   90
#define  LINESIZE  75

char ribFile[FILELEN];				//original Rib file
char ribFile_IP[FILELEN];			//if Rib file is in binary format, transform to IP format

char cTwoHopFile[FILELEN];			//output compressed rib file in IP format: one prefix with two hops
char cOneHopFile[FILELEN];			//output compressed rib file in IP format: one prefix with one hop
char cOneHopFile_bin[FILELEN];		//output compressed rib file in binary format: one prefix with one hop

bool bIpFormat = true;				//mark of Rib file,IP format initially e.g."160.120.228/20 5356"
int  algorithm;						//define which algorithm to use


//get a string of iLen duplicated elements.
string DupString(char cElement, int iLen, bool bNewLine){
	string strRet;
	for (int i = 1; i <= iLen; i++){
		if (i == iLen && bNewLine == true){
			strRet += "\n";
		}
		else{
			strRet += cElement;
		}
	}
	return strRet;
}

//to verify if the file exists
bool Exist(char * FileName)
{
	FILE*   fp;
	if (fopen_s(&fp, FileName, "r") == 0)
	{
		fclose(fp);
		return true;
	}
	else
		return false;
}


void ShowTipInfo()
{
	printf("%s", DupString('=', LINESIZE, true).c_str());
	printf("The program implements the US Algorithm, and outputs the compressed FIB.\n");
	printf("Usage: US.exe [FileName] [Prefix Format] [Output]\n");
	printf("Options:\n");
	printf("[FileName]\t\tThe name of original FIB file. No longer than %u letters.\n", FILELEN);
	printf("[Prefix Format]\t\tB/b - Binary format. I/i - IP format.\n");
	printf("[Output]\t\tO/o - Output the compressed FIBs to file. This parameter is optional.\n");
}

void FIB_UPDATE_ShowTipInfo()
{
	printf("FIB_UPDATE_OUTPUT\n");
	printf("The program implements the update algorithms of binary trie and US, and outputs the updated FIB by different algorithms.\n");
	printf("Usage: FibUpdateOutput.exe [FibFileName] [UpdateFibFileName] [Algorithm] [FIBFormat] [UpdateFileFormat]\n");
	printf("Options:\n");
	printf("[FibFileName]\t The name of original FIB file. No longer than %u letters.\n", FILELEN);
	printf("[UpdateFibFileName]\t The name of Update FIB file. No longer than %u letters.\n", FILELEN);
	printf("[Algorithm]\t  N/n - Normal. U/u - US. A/a - All algorithms.\n");
	printf("[FIBFormat]\t B/b - Binary format. I/i - IP format.\n");
	printf("[UpdateFileFormat]\t If there is a corresponding binary update file, input B/b. Otherwise, input I/i.\n");
}


/*
Evaluate the  update performance 
outputFile: "ribUpdate.stat"
*/
int main_update(int argc, char **argv)
//int main(int argc, char **argv)
{
	if (argc < 6)
	{
		printf("FIB_UPDATE\n");
		printf("The program evaluates the update performance of binary trie and US.\n");
		printf("Usage: FibUpdate.exe [FibFileName] [UpdateFibFileName] [Algorithm] [FIBFormat] [UpdateFileFormat]\n");
		printf("Options:\n");
		printf("[FibFileName]\t The name of original FIB file. No longer than %u letters.\n", FILELEN);
		printf("[UpdateFibFileName]\t The name of Update FIB file. No longer than %u letters.\n", FILELEN);
		printf("[Algorithm]\t  N/n - Normal. U/u - US. A/a - All algorithms.\n");
		printf("[FIBFormat]\t B/b - Binary format. I/i - IP format.\n");
		printf("[UpdateFileFormat]\t If there is a corresponding binary update file, input B/b. Otherwise, input I/i.\n");
		system("pause");
		return 0;
	}	

	if (argc == 6)
	{
		char fib_file[FILELEN];
		char bin_fib_file[FILELEN];
		char update_file[FILELEN];
		char bin_update_file[FILELEN];

		memset(fib_file, 0, sizeof(fib_file));
		memset(bin_fib_file, 0, sizeof(bin_fib_file));
		memset(update_file, 0, sizeof(update_file));
		memset(bin_update_file, 0, sizeof(bin_update_file));
		strcpy_s(fib_file, FILELEN, argv[1]);
		strcpy_s(update_file, FILELEN, argv[2]);
		strcpy_s(bin_fib_file, FILELEN, "Bin_");
		strcat_s(bin_fib_file, fib_file);
		strcpy_s(bin_update_file, FILELEN, "BinUpdate_");
		strcat_s(bin_update_file, update_file);

		char *outPut_file = "ribUpdate.stat";
		if (!Exist(outPut_file))
		{
			ofstream fout(outPut_file);
			fout << "Minute_of_update\t" << "prefixes\t" << "update_time(us)\t" << "avg_memory_access\t" << "updates_per_min\t" << "updateTime_per_min\t" << "update_speed(Mups)\t"<<endl;
			fout.close();
		}

		if (Exist(fib_file) == false)
		{
			printf("\nRIb file \t%s does not exist.\n", fib_file);
			return 0;
		}
		if (Exist(update_file) == false)
		{
			printf("\n srcIP file \t%s does not exist.\n", update_file);
			return 0;
		}

		if (argv[3] != NULL && (argv[3][0] == 'N' || argv[3][0] == 'n'))
			algorithm = BASIC;
		else if (argv[3] != NULL && (argv[3][0] == 'U' || argv[3][0] == 'u'))
			algorithm = US;
		else if (argv[3] != NULL && (argv[3][0] == 'A' || argv[3][0] == 'a'))
			algorithm = ALL;

		if (argv[5][0] == 'I' || argv[5][0] == 'i')
		{
			CFib Fib = CFib();
			Fib.ConvertUpdateFileIpToBin(update_file, bin_update_file);
		}

		char format = argv[4][0];
		ofstream fout(outPut_file, ios::app);
		fout << fib_file << " & " << update_file << endl;

		//*********************** Original Binary trie update  **********************
		if (algorithm == ALL || algorithm == BASIC)
		{
			fout << "BASIC" << endl;
			printf("\nApplying Normal binary trie update algorithm...\n");
			CFib Fib = CFib();
			if (format == 'I' || format == 'i')
			{
				Fib.BuildFibFromFile(fib_file);
			}
			else
			{
				Fib.BuildFibFromBinFile(bin_fib_file);
			}

			Fib.UpdateFromFile2(bin_update_file, BASIC, &fout);
			Fib.ClearTrie(Fib.m_pTrie);
		}//if

		//*********************** US update  **********************
		if (algorithm == ALL || algorithm == US)
		{
			fout << "US" << endl;
			printf("\nApplying US update algorithm...\n");
			CFib Fib = CFib();
			if (format == 'I' || format == 'i')
			{
				Fib.BuildFibFromFile(fib_file);
			}
			else
			{
				Fib.BuildFibFromBinFile(bin_fib_file);
			}
			Fib.CompressTrie();
			Fib.UpdateFromFile2(bin_update_file, US, &fout);
			Fib.ClearTrie(Fib.m_pTrie);
		}//if

		fout.flush();
		fout.close();
	}

	return 0;
}


/*
output the original rib and updated rib by different algorithms
*/
int main_updateOutput(int argc, char **argv)
//int main(int argc, char **argv)
{
	char Normal_file[FILELEN];
	char fib_file[FILELEN];
	char bin_fib_file[FILELEN];
	char update_file[FILELEN];
	char bin_update_file[FILELEN];

	memset(Normal_file, 0, sizeof(Normal_file));
	memset(fib_file, 0, sizeof(fib_file));
	memset(bin_fib_file, 0, sizeof(bin_fib_file));
	memset(update_file, 0, sizeof(update_file));
	memset(bin_update_file, 0, sizeof(bin_update_file));


	if (argc < 6)
	{
		FIB_UPDATE_ShowTipInfo();
		system("pause");
		return 0;
	}

	if (argc == 6)
	{
		strcpy_s(fib_file, FILELEN, argv[1]);
		strcpy_s(update_file, FILELEN, argv[2]);
		strcpy_s(bin_fib_file, FILELEN, "Bin_");
		strcat_s(bin_fib_file, fib_file);
		strcpy_s(bin_update_file, FILELEN, "BinUpdate_");
		strcat_s(bin_update_file, update_file);


		if (Exist(fib_file) == false)
		{
			printf("\nRIb file \t%s does not exist.\n", fib_file);
			return 0;
		}
		if (Exist(update_file) == false)
		{
			printf("\n srcIP file \t%s does not exist.\n", update_file);
			return 0;
		}

		if (argv[3] != NULL && (argv[3][0] == 'N' || argv[3][0] == 'n'))
			algorithm = BASIC;
		else if (argv[3] != NULL && (argv[3][0] == 'U' || argv[3][0] == 'u'))
			algorithm = US;
		else if (argv[3] != NULL && (argv[3][0] == 'A' || argv[3][0] == 'a'))
			algorithm = ALL;

		char format = argv[4][0];

		if (argv[5][0] == 'I' || argv[5][0] == 'i')
		{
			CFib Fib = CFib();
			Fib.ConvertUpdateFileIpToBin(update_file, bin_update_file);
		}

		memset(cTwoHopFile, 0, FILELEN);
		memset(cOneHopFile, 0, FILELEN);

		strcat_s(Normal_file, fib_file);
		strcat_s(Normal_file, "_");
		strcat_s(Normal_file, update_file);
		strcat_s(Normal_file, "_Normal");

		strcat_s(cTwoHopFile, fib_file);
		strcat_s(cTwoHopFile, "_");
		strcat_s(cOneHopFile, fib_file);
		strcat_s(cOneHopFile, "_");
		strcat_s(cTwoHopFile, update_file);
		strcat_s(cOneHopFile, update_file);
		strcat_s(cTwoHopFile, "_c_2hop");
		strcat_s(cOneHopFile, "_c_1hop");

		//*********************** Update of original binary trie  **********************
		if (algorithm == ALL || algorithm == BASIC)
		{
			CFib Fib = CFib();

			printf("\nApplying Normal update algorithm...\n");

			if (format == 'I' || format == 'i')
				Fib.BuildFibFromFile(fib_file);
			else
				Fib.BuildFibFromBinFile(bin_fib_file);

			Fib.UpdateFromFile(bin_update_file, BASIC);
			strcat_s(Normal_file, "_update");
			Fib.OutPutNormalRib(Fib.m_pTrie, Normal_file);
			Fib.ClearTrie(Fib.m_pTrie);
		}//if

		//*********************** US  **********************
		if (algorithm == ALL || algorithm == US)
		{
			CFib Fib = CFib();

			printf("\nApplying US update algorithm...\n");

			if (format == 'I' || format == 'i')
				Fib.BuildFibFromFile(fib_file);
			else
				Fib.BuildFibFromBinFile(bin_fib_file);

			Fib.CompressTrie();
			Fib.UpdateFromFile(bin_update_file, US);
			strcat_s(cTwoHopFile, "_update");
			strcat_s(cOneHopFile, "_update");
			Fib.OutputCompressedTrie(Fib.m_pTrie, cTwoHopFile, cOneHopFile);
			Fib.ClearTrie(Fib.m_pTrie);
		}//if
	}

	//system("pause");
	return 0;
}


/*
GetNodecounts of hybrid lookup solution: # of leaf nodes and trimmed trie nodes
outputFileName: "hybrid.stat"
*/
int main_hybrid(int argc, char **argv)
//int main(int argc, char **argv)
{
	if (argc != 4)
	{
		cout << "usage: FibCount.exe [FibFileName] [Algorithm] [FibFormat]" << endl;
		cout << "[FibFileName]\t The name of original FIB file. No longer than " << FILELEN << " letters" << endl;
		cout << "[Algorithm]\t  N/n - Normal. U/u - US. A/a - All algorithms." << endl;
		cout << "[FibFormat]\t B/b - Binary format. I/i - IP format." << endl;
		system("pause");
		return 0;
	}

	char fib_file[FILELEN];
	char bin_fib_file[FILELEN];
	char outPut_file[] = "hybrid.stat";

	memset(fib_file, 0, sizeof(fib_file));
	memset(bin_fib_file, 0, sizeof(bin_fib_file));

	strcpy_s(fib_file, FILELEN, argv[1]);
	strcpy_s(bin_fib_file, FILELEN, "BinIP_");
	strcat_s(bin_fib_file, fib_file);

	if (Exist(fib_file) == false)
	{
		printf("\nRIb file \t%s does not exist.\n", fib_file);
		system("pause");
		return 0;
	}

	char format = argv[3][0];

	if (!Exist(outPut_file))
	{
		ofstream fout(outPut_file);
		fout << "FileName\t" << "Algorithm\t" << "AllNodes\t" << "PrefixNodes\t" << "LeafNodes\t" << "TrimmedTrieNodes\t" << endl;
		fout.close();
	}

	ofstream fout(outPut_file, ios::app);

	if (argv[2] != NULL && (argv[2][0] == 'N' || argv[2][0] == 'n'))
		algorithm = BASIC;
	else if (argv[2] != NULL && (argv[2][0] == 'U' || argv[2][0] == 'u'))
		algorithm = US;
	else if (argv[2] != NULL && (argv[2][0] == 'A' || argv[2][0] == 'a'))
		algorithm = ALL;

	//*********************** BASIC  **********************
	if (algorithm == ALL || algorithm == BASIC)
	{
		CFib Fib = CFib();

		printf("\nApplying Basic algorithm...\n");

		if (format == 'I' || format == 'i')
			Fib.BuildFibFromFile(fib_file);
		else
			Fib.BuildFibFromBinFile(bin_fib_file);

		Fib.GetNodeCounts2(BASIC);

#ifdef DEBUG
		cout << "Algorithm " << "Total  " << " Prefix " << " Leaf " << " Trimmed " << " LeafDeleteNode " << Fib.leafDeleteNodecount << endl;;
		cout << algorithm << " " << Fib.allNodeCount << " " << Fib.oldNodeCount << " " << Fib.leafNodeCount << " " << Fib.allNodeCount - Fib.leafDeleteNodecount << endl;
#endif
		fout << fib_file << "\t";
		fout << "BASIC\t";
		fout << Fib.allNodeCount << "\t" << Fib.oldNodeCount << "\t" << Fib.leafNodeCount << "\t" << Fib.allNodeCount - Fib.leafDeleteNodecount << endl;
		Fib.ClearTrie(Fib.m_pTrie);
	}//if

	//*********************** US  **********************
	if (algorithm == ALL || algorithm == US)
	{
		printf("\nApplying US algorithm...\n");

		CFib Fib = CFib();

		if (format == 'I' || format == 'i')
			Fib.BuildFibFromFile(fib_file);
		else
			Fib.BuildFibFromBinFile(bin_fib_file);

		Fib.CompressTrie(true);
		Fib.GetNodeCounts2(US);

#ifdef DEBUG
		cout << "Algorithm " << "Total  " << " Prefix " << " Leaf " << " Trimmed " << " LeafDeleteNode " << Fib.leafDeleteNodecount << endl;;
		cout << algorithm << " " << Fib.allNodeCount << " " << Fib.oldNodeCount << " " << Fib.leafNodeCount << " " << Fib.allNodeCount - Fib.leafDeleteNodecount << endl;
#endif
		fout << fib_file << "\t";
		fout << "US\t";
		fout << Fib.allNodeCount << "\t" << Fib.oldNodeCount << "\t" << Fib.leafNodeCount << "\t" << Fib.allNodeCount - Fib.leafDeleteNodecount << endl;
		Fib.ClearTrie(Fib.m_pTrie);
	}//if

	fout.close();
	//system("pause");
	return 0;
}


/*
Implements the US algorithm: Union and Split.
Output:compressed rib files and a statistic file
*/
int main_UScompress(int argc, char * argv[])
//int main(int argc, char * argv[])
{
	if (argc < 3)
	{
		ShowTipInfo();
		system("pause");
		return 0;
	}
	
	memset(ribFile, 0, FILELEN);
	memset(cTwoHopFile, 0, FILELEN);
	memset(cOneHopFile, 0, FILELEN);

	strcpy_s(ribFile, FILELEN, argv[1]);
	strcpy_s(cTwoHopFile, FILELEN, argv[1]);
	strcpy_s(cOneHopFile, FILELEN, argv[1]);

	strcat_s(cTwoHopFile, "_c_2hop");
	strcat_s(cOneHopFile, "_c_1hop");

	//to verify if the file exists
	if (Exist(ribFile) == false)
	{
		printf("\nRIb file \t%s does not exist.\n", ribFile);
		printf("%s", DupString('=', LINESIZE, true).c_str());
		return 0;
	}

	//to verify if the file format is legal
	if (argv[2] == NULL || (argv[2] != NULL && argv[2][0] != 'B' && argv[2][0] != 'b' && argv[2][0] != 'I' && argv[2][0] != 'i'))
	{
		printf("\nInput Error! File can only be in binary format(B) or IP format(I).\n");
		printf("%s", DupString('=', LINESIZE, true).c_str());
		return 0;
	}
	else
	{
		if (argv[2][0] == 'B' || argv[2][0] == 'b')
			bIpFormat = false;
	}

	CFib Fib = CFib();		//build Fib Trie
	unsigned int iEntryCount = 0;

	//read rib file and build trie
	if (bIpFormat)
		iEntryCount = Fib.BuildFibFromFile(ribFile);
	else
		iEntryCount = Fib.BuildFibFromBinFile(ribFile);

	char * CompStat = "US_Compression.stat";

	if (!Exist(CompStat))
	{//create statistics file
		ofstream fout(CompStat);
		fout << "File Name\t"
			<< "Routing Items\t"
			<< "All Nodes\t"
			<< "Original prefixes\t"
			<< "Prefixes after US\t"
			<< "Compression Ratio(%)\t"
			<< "Compression Time(ms)\t"
			<< endl;
		fout << flush;
		fout.close();
	}

	//*********************** US and output **********************
	LARGE_INTEGER frequence,begin,end;
	long timeOverhead;
	if(!QueryPerformanceFrequency(&frequence))return 0;
	
	printf("\nApplying US algorithm...\n");

	QueryPerformanceCounter(&begin);
	Fib.CompressTrie();
	QueryPerformanceCounter(&end);
	timeOverhead = 1000*(end.QuadPart-begin.QuadPart)/frequence.QuadPart;

	//output compressed FIB files
	if (argv[3] != NULL && ('O' == argv[3][0] || 'o' == argv[3][0]))
	{
		printf("Output the US result...\n");
		if (bIpFormat)
			Fib.OutputCompressedTrie(Fib.m_pTrie,cTwoHopFile,cOneHopFile);
		else
			Fib.OutputCompressedTrieBin(Fib.m_pTrie, cTwoHopFile, cOneHopFile);
		printf("Dump the result into the output file!\n\n");
	}

	//output statistical data
	Fib.GetNodeCounts();
	float ratio = (float)Fib.newNodeCount / Fib.oldNodeCount * 100;

	cout << setiosflags(ios::left)
		<< setw(20) << "File Name"
		<< setw(15) << "Routing Items"
		<< setw(15) << "All Nodes"
		<< setw(15) << "Original"
		<< setw(15) << "US"
		<< setw(23) << "US Ratio(%)"
		<< setw(23) << "US Overhead(ms)"<< endl;
	cout << setiosflags(ios::left)
		<< setw(20) << ribFile
		<< setw(15) << iEntryCount
		<< setw(15) << Fib.allNodeCount
		<< setw(15) << Fib.oldNodeCount
		<< setw(15) << Fib.newNodeCount
		<< setw(23) << fixed << setprecision(2) << ratio
		<< setw(23) << timeOverhead << endl;

	ofstream fout(CompStat, ios::app);
	fout << ribFile << "\t"
		<< iEntryCount << "\t"
		<< Fib.allNodeCount << "\t"
		<< Fib.oldNodeCount << "\t"
		<< Fib.newNodeCount << "\t"
		<< fixed << setprecision(2) << ratio << "\t"
		<< timeOverhead << "\t"
		<< endl;
	fout << flush;
	fout.close();
	
	//Exit the program
	printf("%s", DupString('=', LINESIZE, true).c_str());
	return 0;
}


//test the lookup speed of binary trie
void testLookupSpeed_binary(char * traffic_file, char *fib_file)
{
	printf("\n\nOriginal binary trie lookup starts...\n\n");
	CFib tFib = CFib();
	tFib.BuildFibFromFile(fib_file);
	unsigned int *traffic = tFib.TrafficRead(traffic_file);

	register unsigned char LPMPort = 0;
	LARGE_INTEGER frequence, begin, end;
	if (!QueryPerformanceFrequency(&frequence))return;
	printf("\tfrequency=%u\n", frequence.QuadPart);//2825683

	QueryPerformanceCounter(&begin);
	for (int j = 0; j<CYCLEs; j++)
	{
		for (int i = 0; i<TRACE_READ; i++)
		{
			//lookup the input IP address: i.e. traffic[i]
			LPMPort = tFib.lookupOneIP_binary(traffic[i]);
		}
	}
	QueryPerformanceCounter(&end);

	long long Lookuptime = 1000000 * (end.QuadPart - begin.QuadPart) / frequence.QuadPart;
	cout << "\tLMPport=" << (int)LPMPort << "\n\tLookup time=" << Lookuptime << "\n\tThroughput is:\t" << setiosflags(ios::fixed) << setprecision(3) << (CYCLEs + 0.0)*TRACE_READ / Lookuptime << "Mpps" << endl;

	char outPut_file[] = "Throughput.stat";
	ofstream fout(outPut_file, ios::app);
	fout << fib_file << "\t" << traffic_file << "\tBasic\t" << (CYCLEs + 0.0)*TRACE_READ / Lookuptime << "\tMpps" << endl;

	fout.close();
	tFib.ClearTrie(tFib.m_pTrie);
}

//test the lookup speed after US compression
void testLookupSpeed_compress(char * traffic_file, char *fib_file)
{
	printf("\n\nUS lookup starts...\n\n");
	CFib tFib = CFib();

	tFib.BuildFibFromFile(fib_file);
	tFib.CompressTrie();
	tFib.OutputCompressedTrie(tFib.m_pTrie, "compress2hop.trie", "compress1hop.trie");

	USFib tUSFib = USFib();
	tUSFib.BuildFibFromFile("compress2hop.trie");

	unsigned int *traffic = tFib.TrafficRead(traffic_file);

	register unsigned char LPMPort = 0;

	LARGE_INTEGER frequence, begin, end;
	if (!QueryPerformanceFrequency(&frequence))return;
	QueryPerformanceCounter(&begin);
	printf("\tfrequency=%u\n", frequence.QuadPart);//2825683

	for (int j = 0; j<CYCLEs; j++)
	{
		for (int i = 0; i<TRACE_READ; i++)
		{
			LPMPort = tUSFib.lookupOneIP_compress(traffic[i]);
		}
	}

	QueryPerformanceCounter(&end);
	long long Lookuptime = 1000000 * (end.QuadPart - begin.QuadPart) / frequence.QuadPart;
	cout << "\tLMPport=" << (int)LPMPort << "\n\tLookup time=" << Lookuptime << "\n\tThroughput is:\t" << setiosflags(ios::fixed) << setprecision(3) << (CYCLEs + 0.0)*TRACE_READ / Lookuptime << "Mpps" << endl;

	char outPut_file[] = "Throughput.stat";
	ofstream fout(outPut_file, ios::app);
	fout << fib_file << "\t" << traffic_file << "\tUS\t" << (CYCLEs + 0.0)*TRACE_READ / Lookuptime << "\tMpps" << endl;
	fout.close();

	tFib.ClearTrie(tFib.m_pTrie);
}

//Test if the lookup result of the compressed trie is the same as that of the original binary trie
void test_correctness(char *fib_file)
{
	CFib tFib = CFib();
	tFib.BuildFibFromFile(fib_file);
	tFib.CompressTrie();
	tFib.OutputCompressedTrie(tFib.m_pTrie, "compress2hop.trie", "compress1hop.trie");

	USFib tUSFib = USFib();
	tUSFib.BuildFibFromFile("compress2hop.trie");

	unsigned int port1 = 0;
	unsigned int port2 = 0;
	unsigned int IP = 0;
	for (; IP<4294967295; IP++)
	{
		port1 = tFib.lookupOneIP_binary(IP);
		port2 = tUSFib.lookupOneIP_compress(IP);

		if (port1 != port2)
		{
			printf("\nnot equal...IP: %u\tBinary trie nexthop: %d\tCompressed trie nexthop: %d\n", IP, port1, port2);
			system("pause");
			return;
		}
		if (0 == IP % 10000000)
		{
			printf("\r%u    %d    %d    %.3f%%          ", IP, port1, port2, 100.0*IP / 4294967295);
		}
	}
	printf("Congratulations! Equal! Mission complete!\n");
	system("pause");
}

/*
Implements the lookup algorithms of the binary trie and US. 
Output the throughput statistic file
*/
int main_lookup(int argc, char **argv)
//int main(int argc, char **argv)
{
	if (argc != 4)
	{
		cout << "usage: FibLookup.exe [Algorithm] [FibFileName] [TrafficFileName]" << endl;
		cout << "[Algorithm]\t  B/b - Basic. U/u - US. A/a - All algorithms." << endl;
		cout << "[FibFileName]\t The name of original FIB file. No longer than " << FILELEN << " letters" << endl;
		cout << "[TrafficFileName]\t The name of traffic file. No longer than " << FILELEN << " letters" << endl;

		system("pause");
		return 0;
	}

	//test_correctness(argv[2]);
	//return 0;

	char fib_file[FILELEN];
	char traffic_file[FILELEN];
	char outPut_file[] = "Throughput.stat";

	memset(fib_file, 0, sizeof(fib_file));
	memset(traffic_file, 0, sizeof(traffic_file));

	strcpy_s(fib_file, FILELEN, argv[2]);
	strcpy_s(traffic_file, FILELEN, argv[3]);

	if (Exist(fib_file) == false)
	{
		printf("\nRIb file \t%s does not exist.\n", fib_file);
		system("pause");
		return 0;
	}

	if (Exist(traffic_file) == false)
	{
		printf("\nTraffic file \t%s does not exist.\n", traffic_file);
		system("pause");
		return 0;
	}

	if (!Exist(outPut_file))
	{
		ofstream fout(outPut_file);
		fout << "FileName\t" << "TrafficFileName\t" << "algorithm\t" << "throughput(Mpps)\t" << endl;
		fout.close();
	}

	if (argv[1] != NULL && (argv[1][0] == 'B' || argv[1][0] == 'b'))
		algorithm = BASIC;
	else if (argv[1] != NULL && (argv[1][0] == 'U' || argv[1][0] == 'u'))
		algorithm = US;
	else if (argv[1] != NULL && (argv[1][0] == 'A' || argv[1][0] == 'a'))
		algorithm = ALL;

	if (algorithm == ALL || algorithm == BASIC)
	{
		printf("\nApplying Basic lookup algorithm...\n");
		testLookupSpeed_binary(traffic_file, fib_file);
	}//

	if (algorithm == ALL || algorithm == US)
	{
		printf("\nApplying US lookup algorithm...\n");
		testLookupSpeed_compress(traffic_file, fib_file);

	}//if

	//system("pause");
	return 0;
}