---
title: "LFI fuzzer"
categories:
  - Pentest
tags:
  - Python, File Inclusion, Webapp
---


Some time ago I completed [TryHackMe's File Inclusion, Path Traversal room](https://tryhackme.com/r/room/filepathtraversal). This room is pretty handy to try and work LFI/RFI on a PHP webapp.

Until then, I was pretty used to detect and exploit LFI by hand, or as often seen, with a general purpose fuzzer such as FFUF.

Nothing wrong about FFUF, but since it is a general purpose fuzzer, it will only send "dumb" requests based on a wordlist, so the effectiveness of your pentest will mostly rely on how good your wordlist is.

One can find many different wordlists [here](https://github.com/carlospolop/Auto_Wordlists/blob/main/wordlists/file_inclusion_linux.txt), [here](https://github.com/emadshanab/LFI-Payload-List/blob/master/LFI%20payloads.txt) or [there](https://github.com/DragonJAR/Security-Wordlist/blob/main/LFI-WordList-Linux)

Some wordlists only contains absolute paths to useful files, but then it is a pain to adapt the payload : find the good path traversal, encoding, may be apply a PHP filter ?

Other wordlists include path traversal and encodings, but these may not be consistent and it's hard to be sure every variations for every possible file is present. Also, this kind of wordlist is easily bloated from so many variations and would be hard to maintain.

Acknowledging these issues, the idea is to have a specific, simple tool to LFI/RFI fuzzing, with enough options to be adequate to most needs, and easy enough to XXXX

Enough words of introduction, let's highlight some key features. A complete list of arguments is available on the [LFI fuzzer repo](https://github.com/20100dbg/Python-Stuff/tree/master/LFI_fuzzer)


Fuzz parameters
prefix

a (weak) security against LFI attacks is to check whether the include path begins with 

traversal-method
to navigate

encode

stress /encode

success check

common checks would be 

-es "page not found"
-l XXX where XXX is 

for example, /etc/passwd should contains "root:x:0:0"
and c:\windows\win.ini should contains "for 16-bit app support"