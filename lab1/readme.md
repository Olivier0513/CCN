# Socket Programming
## 實現TCP檔案傳輸
### server端前置準備：
1. 使用socket(AF_INET, SOCK_STREAM, 0)建立socket。
2. 設定server的sin_family, sin_addr以及sin_port。
3. 使用上方資訊進行bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) 。
4. 透過listen()進入被動監聽狀態。
5. 透過accept建立新的通道，等待有client要求連線。
### client端前置準備：
1. 使用socket(AF_INET, SOCK_STREAM, 0)建立socket。
2. 透過gethostbyname()取得host資訊。
3. 設定server的sin_family, sin_addr以及sin_port。
4. 使用上方資訊進行connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))。
### server-client檔案開始傳輸：
1. 在connect成功後，server會先將檔案名稱透過send()傳給client（client端使用recv()接收）。
2. server端打開檔案後，透過fseek()去計算檔案大小，然後send給client。
3. 接著server端會開始send要傳送的資料，直到檔案全部傳送完成。
4. 兩邊傳送結束後close socket，完成本次傳輸。
## 計算傳送時間
- 透過clock_t宣告t1, t2，在client開始接收前紀錄t1，接收完成後紀錄t2，兩者相減可得傳輸時間：
![](https://i.imgur.com/lUnwkS6.png)
## 計算傳送進度
- 宣告total_length去紀錄目前檔案傳輸的Bytes，然後跟總檔案大小做比較，每達到25%就紀錄一次。
![](https://i.imgur.com/hUMx78t.png)
## 實際傳送結果(TCP)
- server執行: ./lab1_file_transfer tcp send \<ip> \<port> test_input.txt
- client執行: ./lab1_file_transfer tcp send \<ip> \<port> 
![](https://i.imgur.com/mWmzvN5.png)
## 實現UPD檔案傳輸
### server端前置準備：
1. 使用socket(AF_INET, SOCK_STREAM, 0)建立socket。
2. 設定server的sin_family, sin_addr以及sin_port。
3. 使用上方資訊進行bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) 。
4. 透過recvfrom()等待client傳送訊息。
### client端前置準備：
1. 使用socket(AF_INET, SOCK_STREAM, 0)建立socket。
2. 透過gethostbyname()取得host資訊。
3. 設定server的sin_family, sin_addr以及sin_port。
4. 使用sendto()傳送自身address訊息給server。
### server-client檔案開始傳輸：
1. 在receive client訊息後，server會依序傳送檔案名稱及大小給client（和TCP傳送的內容相同）。
2. 接著server端會開始send要傳送的資料，直到檔案全部傳送完成。
3. 兩邊傳送結束後，因為UDP沒有connect結束可以判斷，因此使用特殊字串來判斷傳輸結束（詳細見下張投影片）。
4. 最後用接收檔案大小跟原始檔案大小比較得到最後的packet loss rate。
## UDP檔案傳輸判斷
- 在server端傳輸結束後，傳送”UDP_file_send_ended”給client，因為UDP的特性是可能會遺失，因此傳送數次以確保有傳輸成功。client端讀取到則break出迴圈：
![](https://i.imgur.com/Pce2aQO.png)
![](https://i.imgur.com/BzTCAV3.png)
## 實際傳送結果(UDP)
- server執行: ./lab1_file_transfer udp send \<ip> \<port> test_input.txt
- client執行: ./lab1_file_transfer udp send \<ip> \<port> 
![](https://i.imgur.com/0p2SFL9.png)