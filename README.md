SteamCrawler
============

Used Libraries

- Boost 1.55.0
- libcurl
- gumbo-parser
- MySQL C++ Connector
- rapidjson

Need To be Added

- Crawl Game Informations
- Multi-Threading
- Make Parsing Job more efficiently

Abandone current crawler for better scalability
=============
+ Make crawler to crawl every anchor tag and put it on the DB
+ Need to think how to manage these urls..
+ Fetch the url and if the html pattern matches the wanted page, process(parse) it!
