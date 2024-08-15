---
title: "LFI fuzzer"
categories:
  - Pentest
tags:
  - Python
  - Webapp
  - File Inclusion
---


### Why another fuzzer ?

Some time ago I completed [TryHackMe's File Inclusion, Path Traversal room](https://tryhackme.com/r/room/filepathtraversal). This room is pretty handy to try and work LFI/RFI on a PHP webapp.

Until then, I was pretty used to detect and exploit LFI by hand, or as often seen, with a general purpose fuzzer such as FFUF.

Nothing wrong about FFUF, but since it is a general purpose fuzzer, it will only send "dumb" requests based on a wordlist, so the effectiveness of your pentest will mostly rely on how good your wordlist is.

One can find many different wordlists [here](https://github.com/carlospolop/Auto_Wordlists/blob/main/wordlists/file_inclusion_linux.txt), [here](https://github.com/emadshanab/LFI-Payload-List/blob/master/LFI%20payloads.txt) or [there](https://github.com/DragonJAR/Security-Wordlist/blob/main/LFI-WordList-Linux)

Some wordlists only contains absolute paths to useful files, but then it is a pain to adapt the payload : how to find the good path traversal, encoding and other evasion techniques ?

Other wordlists include path traversal and encodings, but these may not be consistent and it's hard to be sure every variations for every possible file is present. Also, this kind of wordlist is easily bloated from so many variations and would be hard to maintain.

Acknowledging these issues, the idea is to have a specific, simple tool to LFI/RFI fuzzing, with enough options to be adequate to most needs.

Enough words of introduction, let's highlight some key features. A complete list of arguments is available on the [LFI fuzzer repo](https://github.com/20100dbg/Python-Stuff/tree/master/LFI_fuzzer)


### Features

User needs to provide a URL and a wordlist. In current version, URL is expected to be something like `http://victim.com/?page=` and payloads will be appended to it.

##### Fuzz parameters
- nb-parents : how many directories to traverse
- encode : encode 0, 1 or 2 times
- path-prefix : prefix payload with a required folder or php filter 
- append-null : add a null byte at the end of the payload
- traversal-method : will use ../ or ..// or ....// or randomly generated
- dir-separator : will use / or \ or \\ or /./
- var : replace `[VAR]` placeholder in some payloads with given value

##### Some other parameters
- stress : tries every variation for every parameter on each item of a given wordlist
- download : save result of successful attempts


##### Sucess/error detection

Ignore if a result length is < 1300
```
--min-length 1300
```

Looking for a specific string in files
/etc/passwd should contains `root:x:0:0`
```
--success-string "root:x:0:0"
```

c:\windows\win.ini should contains "for 16-bit app support"
```
--success-string "for 16-bit app support"
```

Possible error message
```
--error-string "page not found"
```


### Possible future features

- Support payload anywhere in URL, in POST data, cookies and headers
- Extend `--var` parameter to support a wordlist
- More encoding options

That's all folks ! I hope you'll find this tool as useful as I do !
