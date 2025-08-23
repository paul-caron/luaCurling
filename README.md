# lua_curling
A lua binding of my C++ curling library. Enables a user to make some HTTP requests, inside a REPL.


```lua
lua > req = Request.new() \
       :setURL("https://www.example.com") \
       :setMethod(HttpMethod.GET) \
res = req.send(1) \
print(res:toString())
...
```
