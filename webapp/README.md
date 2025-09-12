## Web app

Dashboard web app for the 18650 tester project. It's a Vite React SPA project.

### Install

```
npm install
```

### Compile

Add ".env" file with a `VITE_ARDUINO_IDS` variable. Eg.

```
VITE_ARDUINO_IDS="123,124"
```

"123,124" means two arduino wifi servers running on 192.168.1.123 and 192.168.1.124 resp.

Then, build the app.

```
npm run build
```

### Run server

Copy dist folder to folder "label-printer" and run together with the label printer.
Or start in dev mode without label printer here:

```
npm run dev
```
