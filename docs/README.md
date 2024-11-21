## Project Specification and Documentation

### Project proposal

**Topic:** videostreaming client-server app

In our scenario we can imagine 3 entities:
 1. video encoder
 2. video-optimized HTTP storage server
 3. video player (client)

Nice feature would be, that the server could provide data that had just been written to it. The video could be streamed to other users even though it hasn't been fully uploaded.

Server be **read heavy**. Support thousands of video players.

#### Implementation should contain
 1. **HTTP PUT**
    - correct writing of data to a given URL
    - written data should be ready for GET method
    - support for *Transfer-Encoding: chunked*
 2. **HTTP GET**
    - corrent download from given URL
    - support for *Transfer-Encoding" chunked*
 3. **HTTP DELETE**
    - remove file at give URL

The project could be simplified by using additional libraries, but I have to do additional research for that. Something like **ffmpeg** could come in handy when developing the video encoder.
