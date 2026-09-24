---
title: "Porting json-server in Go"
categories:
  - dev
tags:
  - go
  - API
classes: wide
---

Some time ago, I stumbled upon [json-server](<https://github.com/typicode/json-server>), a Node.js project that reads a JSON file and exposes its contents through a REST API.

Given a JSON file like this:

```json
{
    "users": [
        {
          "name": "Mario",
          "id": 1
        },
        {
          "name": "Luigi",
          "id": 2
        }
    ]
}
```

Without any additional configuration or development, you can get an HTTP server that lets you read, add, update, and delete items from this collection through REST API calls (here using curl):

```bash
# Show an entry
curl -X GET "http://localhost:8080/api/blogs/1"

# Add an entry
curl -X POST "http://localhost:8080/api/blogs" -d '{"id": 4, "title":"new title", "body":"new body", "author":"new author"}'

# Delete an entry
curl -X DELETE "http://localhost:8080/api/blogs/3"

```

## Translate to Go

This is all good, but there is at least one problem with this project: it requires Node.js, and I still have Vietnam flashbacks from 3 GB `node_modules` folders (okay, in the case of `json-server`, it's actually only about 3.1 MB).

I initially thought about implementing my own json-server in Python, which I like and am pretty comfortable with, but users would have to deal with dependencies, maybe even install Python.

An ideal solution would be a single binary, simple to install, simple to use. It also seemed like a good excuse to finally give Go a try.

The project is on my github: [https://github.com/20100dbg/json-server](https://github.com/20100dbg/json-server)


## Cool stuff

Creating a web server in Go is surprisingly easy. For example, if you need to serve static files:

```go
http.FileServer(http.Dir("./static"))
```

We can also deal with more complex routing using `ServeMux`:

```go
mux := http.NewServeMux()

//mux.HandleFunc("[METHOD] /route/{parameter}", callbackFunction)
mux.HandleFunc("GET /api/{resource}/{id}", getItem)
mux.HandleFunc("POST /api/{resource}", createItem)
mux.HandleFunc("/something", handleSomething)
```

The callback function has access to the HTTP query object and can read the full URL, method, data body, etc for further processing.


```go
func readFile(filePath string) ([]byte, error) {
data, err := os.ReadFile(filePath)
if err != nil {
    return nil, err
}

return data, nil
}
```


## Not-so-cool stuff

#### Working with arbitrary JSON is a pain.

Because `json-server` needs to work with JSON files whose schema is unknown, we can't simply define a Go struct and let `encoding/json` do all the work. Instead, we need to use a collection of any values
and perform type assertions and validation.

#### More specifically: numbers

When JSON is decoded into map[string]any using Go's default encoding/json behavior, JSON numbers are represented as float64. 
We need to use a specific JSON decoder option (`decoder.UseNumber()`) to ensures that JSON numbers are stored as json.Number.

This preserves the original numeric value as a string-like representation instead of converting it to float64. We can then explicitly call Int64() to validate that the value is an integer and safely convert it to int64.

So the flow is:
any -> json.Number -> int64


#### Check verbosity

As I said earlier, error handling makes code feel safe, but because we are handling arbritrary JSON, something simple like find an item by its ID turns into a festival of checks.

Here is a shortened version of the findItemIndex function:

```go
func findItemIndex() {
    // Is the ID parameter an integer?
    id, err := strconv.ParseInt(idString, 10, 64)

    // Does the requested collection exist in the JSON file?
    collection, exists := db[resource]

    // Is the collection an array?
    items, ok := collection.([]any)

    // Browse each item in the collection.
    for idx, item := range items {

        // Is the item an object?
        obj, ok := item.(map[string]any)

        // Finally: does the object contain an "id" field,
        // and does it match the requested ID?
        if currentID, ok := getID(obj); ok && currentID == id {
            return idx, id, http.StatusOK, ""
        }
    }
}
```

## Conclusion

I am still split about working with Go. I am obviously not familiar with syntax and idioms yet. And maybe I need to find a more fitting IDE and plugins. In the end it wasn't a great experience. Not terrible, just meh.

The amount of checks makes the code feel safe and should prevent it from crashing because of a weirdly shaped JSON file. But I guess you never know, so don't use it for anything serious.
