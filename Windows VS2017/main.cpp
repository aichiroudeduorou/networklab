// StopWait.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNRdtSender.h"
#include "GBNRdtReceiver.h"
#include "SRRdtSender.h"
#include "SRRdtReceiver.h"
#include "TCPRdtSender.h"
#include "TCPRdtReceiver.h"

int main(char argc, char* argv[]) {
	RdtSender* ps;
	RdtReceiver* pr;
	if (argc == 2) {
		if (strcmp(argv[1], "GBN") == 0) {
			ps = new GBNRdtSender(4, 3);
			pr = new GBNRdtReceiver(3);
			printf("-***- Protocol: GBN -***-\n\n");
		}
		else if (strcmp(argv[1], "SR") == 0) {
			ps = new SRRdtSender(4, 3);
			pr = new SRRdtReceiver(4, 3);
			printf("-***- Protocol: SR -***-\n\n");
		}
		else if (strcmp(argv[1], "TCP") == 0) {
			ps = new TCPRdtSender(4, 3);
			pr = new TCPRdtReceiver(3);
			printf("-***- Protocol: TCP -***-\n\n");
		}
		else {
			printf("Protocol error!\n\n");
			return 0;
		}
	}
	else {
		ps = new StopWaitRdtSender();
		pr = new StopWaitRdtReceiver();
		printf("-***- StopWait -***-\n\n");
	}
	pns->setRunMode(1);  //安静模式
	//pns->setRunMode(0);   ////VERBOS模式
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");
	pns->start();
	delete ps;
	delete pr;
	delete pUtils;                                  //指向唯一的工具类实例，只在main函数结束前delete
	delete pns;                                     //指向唯一的模拟网络环境类实例，只在main函数结束前delete
	return 0;
}

