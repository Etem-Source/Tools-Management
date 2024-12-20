# 🔨 Tools Management (Mini Project)
This project allows you to store tools, while authenticating with an RFID card and a secret code associated with it.
A secure space, allowing you to track tools.
(All this with a database, and a website (Admin/Client) hosted on a Raspberry PI 4)

## 💡 Communication

![image](https://github.com/user-attachments/assets/750500a0-4993-41d6-9772-5698fab2ef6e)

Diagram explaining the communication between the different components.

## 📚 Website

Login page : 
![image](https://github.com/user-attachments/assets/106a69d4-da4e-43e0-beb8-8ec632f5df83)

Tool inventory :
![image](https://github.com/user-attachments/assets/199e07bf-1be7-4264-8547-9e553472700f)

Who we are ?
![image](https://github.com/user-attachments/assets/b835a0ba-a306-4965-ac4b-bf5408a263c7)

Contact us :
![image](https://github.com/user-attachments/assets/d360fd57-55ca-45e0-996a-69505d1345c3)

Admin page :
![image](https://github.com/user-attachments/assets/995131c6-9544-44a0-bb59-96b8ec13cbbf)

## 🧱 What you need ?

* x1 [Arduino Mega](https://store.arduino.cc/en-fr/products/arduino-mega-2560-rev3?srsltid=AfmBOoqeIRDclvCZ3mwuGdxvQDUXng6BXxADsGaxNTZuQdYlJLhAzPdk)
* x1 [ESP 8266](https://www.amazon.com/ESP8266/s?k=ESP8266)
* x1 [Raspberry PI 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)
* x1 [LCD Screen](https://www.amazon.com/Hosyond-Display-Module-Arduino-Raspberry/dp/B0BWTFN9WF/ref=sr_1_2?dib=eyJ2IjoiMSJ9.P_lTWJsjIDz-81FdySnI8FwjinWYJvFITA9aW3c6wtfUaBFQrqY9xYwa3UsLiAjfZcfsvin2dn6wyT8ujtS9jHQXw_zgMpGy9UXwAgUYUAKQI2wjzy8z6RG6qf6e-Oij3YPzRtycqiQryBzFGlIjy4mjY2CzYwi8cHsFRaWxWfMb9yf9V85_clo4LBhb6GH8RpF0mKqYgbxlDLOMNaeJa841vwmyCOboQE150M8BGx0.3_BMqU-zxmyFR3fw8FV4OffVigO475STaKJ65YPwwhI&dib_tag=se&keywords=LCD%2BDisplay&qid=1734679378&sr=8-2&th=1)
* x1 [RFID Reader](https://www.amazon.fr/AZDelivery-lecteur-Arduino-Raspberry-d%C3%A9marrage/dp/B01M28JAAZ/ref=asc_df_B01M28JAAZ?mcid=f16ef019fd76329386aff45d9006eaa8&tag=googshopfr-21&linkCode=df0&hvadid=701664279035&hvpos=&hvnetw=g&hvrand=15333215830551209119&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9196967&hvtargid=pla-601826090110&psc=1&gad_source=1)
* x1 [Numeric Keypad](https://www.amazon.fr/DollaTek-Universial-Touches-Clavier-Arduino/dp/B07DK57KVM/ref=asc_df_B07DK57KVM?mcid=e714aa90167d3fd6888ee74a40d68924&tag=googshopfr-21&linkCode=df0&hvadid=701590676807&hvpos=&hvnetw=g&hvrand=5979819135847984682&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9196967&hvtargid=pla-1322106490698&psc=1&gad_source=1)
* x1 [Strain Gauge](https://www.gotronic.fr/art-capteur-de-force-1-kg-ampli-hx711-01-34888.htm?gad_source=1&gclid=CjwKCAiApY-7BhBjEiwAQMrrEdG4XzUkz8ARNyx1iYuEfVfWf_tM-cTn4DeiUB6oj3uVO4QsWbbsgBoCACwQAvD_BwE)
* x2 [Distance Sensor](https://www.otronic.nl/fr/capteur-de-distance-a-ultrasons-hc-sr04.html?source=googlebase&gad_source=1&gclid=CjwKCAiApY-7BhBjEiwAQMrrEQoAE9Rq1o442hV21PNTCDdkEVVGzD38iQaJcNAdQcjmLMB16dpsaRoCdcsQAvD_BwE)
* x1 [Servo motor](https://www.amazon.fr/AZDelivery-compatible-H%C3%A9licopt%C3%A8re-Raspberry-Incluant/dp/B07CZ42862/ref=asc_df_B07CZ42862?mcid=241ba1a94422311b8ab6e017f4b253a5&tag=googshopfr-21&linkCode=df0&hvadid=701482302360&hvpos=&hvnetw=g&hvrand=15806183857990792006&hvpone=&hvptwo=&hvqmt=&hvdev=c&hvdvcmdl=&hvlocint=&hvlocphy=9196967&hvtargid=pla-828702003009&gad_source=1&th=1)
* x1 [Radio Module](https://fr.aliexpress.com/item/1005006126995090.html?src=google&pdp_npi=4%40dis%21EUR%212.29%212.29%21%21%21%21%21%40%2112000035876627716%21ppc%21%21%21&src=google&albch=shopping&acnt=248-630-5778&slnk=&plac=&mtctp=&albbt=Google_7_shopping&gclsrc=aw.ds&albagn=888888&ds_e_adid=&ds_e_matchtype=&ds_e_device=c&ds_e_network=x&ds_e_product_group_id=&ds_e_product_id=fr1005006126995090&ds_e_product_merchant_id=107807731&ds_e_product_country=FR&ds_e_product_language=fr&ds_e_product_channel=online&ds_e_product_store_id=&ds_url_v=2&albcp=20180143335&albag=&isSmbAutoCall=false&needSmbHouyi=false&gad_source=1&gclid=CjwKCAiApY-7BhBjEiwAQMrrEc9jQDa8so44ax3WEAuZGbLwEGow79kOMg77PSJYdH_8tuH8I8sZ4hoCorQQAvD_BwE&aff_fcid=56c4a135e64b4952bbc439faccdb87eb-1734679787803-09550-UneMJZVf&aff_fsk=UneMJZVf&aff_platform=aaf&sk=UneMJZVf&aff_trace_key=56c4a135e64b4952bbc439faccdb87eb-1734679787803-09550-UneMJZVf&terminal_id=50a7e5f4c92d4737949521440f8bdc1e&afSmartRedirect=n)


## 📧 Credits
* [Etem](https://github.com/Etem-Source)
* [Luka](https://github.com/Luka-Pereira)
* [Haythem](https://github.com/Haythemchet)
* Corentin
