#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>
#include <fstream>
#include <iostream>
#include <string>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
//定义文件类型对应的content-tyoe
struct doc_type {
    const char* suffix;
    const char* type;
};

//文件
struct doc_type file_type[] =
{
    {"html",  "text/html"},
    {"gif",   "imag/gif"},
    {"jpeg",  "imag/jpeg"},
    {"jpg",   "imag/jpeg"},
    {NULL,    NULL}
};


//响应首部内容
const char* http_res_hdr_tmp1 = "HTTP/1.1 200 OK \r\nServer:FZJ Server<0.1>\r\n"
"Accept-Ranges:bytes\r\nContent-Length:%d\r\nConnection:close\r\n"
"Content-Type:%s\r\n\r\n";
const char* http_res_404 = "HTTP/1.1 404 Not Found \r\nServer:FZJ Server<0.1>\r\n"
"Accept-Ranges:bytes\r\nContent-Length:%d\r\nConnection:close\r\n"
"Content-Type:%s\r\n\r\n";
string ip;
string port;
string root;

void init_env() {
    // 以读模式打开文件
    ifstream infile;
    infile.open("settings.txt");
    cout << "init server" << endl;
    infile >> ip;
    infile >> port;
    infile >> root;
    cout << "listen on:" << ip << ":" << port << endl;
    cout << "root:" << root << endl;
}

//通过后缀，查找到对应的content-type
const char* http_get_type_by_suffix(const char* suffix)
{
    struct doc_type* type;
    for (type = file_type; type->suffix; type++)
    {
        if (strcmp(type->suffix, suffix) == 0)
            return type->type;
    }
    return NULL;
}


//解析客户端发送过来的请求
void http_parse_request_cmd(char* buf, int buflen, char* file_name, char* suffix)
{
    int length = 0;
    char* begin, * end, * bias;

    //查找URL开始位置
    begin = strchr(buf, ' ');
    begin++;

    //查找URL结束位置
    end = strchr(begin, ' ');
    *end = 0;

    bias = strrchr(begin, '/');
    length = end - bias;

    //找到文件名开始的位置
    if ((*bias == '/') || (*bias == '\\'))
    {
        bias++;
        length--;
    }

    //得到客户端请求的文件名
    if (length > 0)
    {
        memcpy(file_name, bias, length);
        file_name[length] = 0;

        begin = strchr(file_name, '.');
        if (begin)
            strcpy(suffix, begin + 1);
    }
}

int http_send_response(SOCKET soc, char* buf, int buf_len)
{
    int read_len, file_len, hdr_len, send_len;
    const char* type;
    char read_buf[1024];
    char http_header[1024];
    char file_name[256] = "index.html";
    char suffix[16] = "html";
    FILE* res_file;

    //通过解析URL，得到文件名
    http_parse_request_cmd(buf, buf_len, file_name, suffix);
    string file = root+"\\";
    file += file_name;
    //打开文件
    cout << "[Web]用户请求:" << file << endl;
    res_file = fopen(file.c_str(), "rb+");
    auto flag_404 = false;
    if (res_file == NULL)
    {
        flag_404 = true;
        res_file= fopen("404.html", "rb+");
		if (strcmp(file_name, "404.html") != 0)
            printf("[Web]文件:%s 不存在!\n", file_name);
    }

    //计算文件大小
    fseek(res_file, 0, SEEK_END);
    file_len = ftell(res_file);
    fseek(res_file, 0, SEEK_SET);

    //获得文件content-type
    type = http_get_type_by_suffix(suffix);
    
    if (type == NULL)
    {
        printf("[Web]没有相关的文件类型!\n");
        return 0;
    }

    //构造响应首部，加入文件长度，content-type信息
    if (flag_404) {
        hdr_len = sprintf(http_header, http_res_404, file_len, type);
    }
    else {
        hdr_len = sprintf(http_header, http_res_hdr_tmp1, file_len, type);
    }
    
    send_len = send(soc, http_header, hdr_len, 0);

    if (send_len == SOCKET_ERROR)
    {
        fclose(res_file);
        printf("[Web]发送失败，错误:%d\n", WSAGetLastError());
        return 0;
    }

    //发送文件
    do
    {
        read_len = fread(read_buf, sizeof(char), 1024, res_file);
        if (read_len > 0)
        {
            send_len = send(soc, read_buf, read_len, 0);
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));
    fclose(res_file);
    return 1;
}
int main() {
    init_env();
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 0), &wsa);
    SOCKET serversoc, acceptsoc;
    SOCKADDR_IN serveraddr;
    SOCKADDR_IN fromaddr;
    char Recv_buf[1024];
    int from_len = sizeof(fromaddr);
    int result;
    int Recv_len;

    //创建socket
    serversoc = socket(AF_INET, SOCK_STREAM, 0);
    if (serversoc == INVALID_SOCKET)
    {
        printf("[Web]创建套接字失败!");
        return -1;
    }

    //初始化服务器IP,Port
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(stoi(port));
    serveraddr.sin_addr.s_addr = inet_addr(ip.c_str());

    //绑定socket
    result = bind(serversoc, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (result == SOCKET_ERROR)
    {
        closesocket(serversoc);
        printf("[Web]绑定套接字失败!");
        return -1;
    }

    //监听socket请求
    result = listen(serversoc, 3);
    printf("[Web]服务器正在运行.....\n");

    while (1)
    {
        //接收请求
        acceptsoc = accept(serversoc, (SOCKADDR*)&fromaddr, &from_len);
        if (acceptsoc == INVALID_SOCKET)
        {
            printf("[Web]接收请求失败!");
            break;
        }
        printf("[Web]连接来自 IP:  %s  Port:  %d  \n", inet_ntoa(fromaddr.sin_addr), ntohs(fromaddr.sin_port));

        //接收来自客户端的请求
        Recv_len = recv(acceptsoc, Recv_buf, 1024, 0);
        if (Recv_len == SOCKET_ERROR)
        {
            printf("[Web]接收数据失败!");
            break;
        }
        Recv_buf[Recv_len] = 0;
        //向客户端发送响应数据
        result = http_send_response(acceptsoc, Recv_buf, Recv_len);

        closesocket(acceptsoc);
    }

    closesocket(serversoc);
    WSACleanup();

    return 0;
}

