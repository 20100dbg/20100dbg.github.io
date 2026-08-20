---
title: "LFI fuzzer"
categories:
  - Pentest
tags:
  - Python
  - Webapp
  - File Inclusion
classes: wide
---

### Update

I finally updated this fuzzer to give it cleaner output and additional parameters, mostly inspired by [ffuf](https://github.com/ffuf/ffuf). The new options include HTTP parameters and filtering options, bringing the fuzzer a bit closer to the features and workflow of ffuf while keeping its main focus on LFI/RFI fuzzing.


### Why another fuzzer ?

Some time ago, I completed [TryHackMe's File Inclusion, Path Traversal room](https://tryhackme.com/r/room/filepathtraversal). This room is pretty handy for learning and practicing LFI/RFI on a PHP web app.

Until then, I was pretty used to detecting and exploiting LFI by hand, or, as is often the case, with a general-purpose fuzzer such as FFUF.

There's nothing wrong with FFUF, but since it is a general-purpose fuzzer, it will only send "dumb" requests based on a wordlist. As a result, the effectiveness of your pentest will mostly depend on how good your wordlist is.

You can find many different wordlists [here](https://github.com/carlospolop/Auto_Wordlists/blob/main/wordlists/file_inclusion_linux.txt), [here](https://github.com/emadshanab/LFI-Payload-List/blob/master/LFI%20payloads.txt), or [there](https://github.com/DragonJAR/Security-Wordlist/blob/main/LFI-WordList-Linux).

Some wordlists only contain absolute paths to useful files, but then it's a pain to adapt the payload: how do you find the right path traversal, encoding, and other evasion techniques?

Other wordlists include path traversal techniques and encodings, but these may not be consistent, and it's hard to be sure that every variation for every possible file is present. Also, this kind of wordlist can easily become bloated with so many variations, making it difficult to maintain.

Taking these issues into account, the idea is to have a specific, simple tool for LFI/RFI fuzzing, with enough options to cover most use cases.

Enough words of introduction. Let's highlight some key features. A complete list of arguments is available in the [LFI fuzzer repository](https://github.com/20100dbg/Python-Stuff/tree/master/LFI_fuzzer).


### Features

We need to provide URL and wordlist as arguments, and use the **LFI** keyword that indicates where to insert the payload.
The LFI keyword may be places in URL, headers, cookies, or POST data.


##### HTTP parameters
lfi_fuzzer supports:
- headers
- cookies
- method (GET or POST)
- POST data
- timeout
- proxy


##### Fuzz parameters
- depth: how many directories to traverse
- encode: (0-5) various URL encoding modes
- prefix : prefix payload (ex: directory name or php filter)
- null : add a null byte at the end of the payload
- variant: will use ../ or ..// or ....// or randomly generated
- separator : will use / or \ or \\\\ or /./
- var : replace `[VAR]` placeholder in with given value

##### Some other parameters
- stress : tries every variation for every parameter on each item of a given wordlist
- download : save result of successful attempts


##### Matching and filtering options

They are pretty much the same 


### Possible future features
- Extend `--var` parameter to support a wordlist
