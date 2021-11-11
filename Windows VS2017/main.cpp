// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
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
	pns->setRunMode(1);  //����ģʽ
	//pns->setRunMode(0);   ////VERBOSģʽ
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");
	pns->start();
	delete ps;
	delete pr;
	delete pUtils;                                  //ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;                                     //ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	return 0;
}

