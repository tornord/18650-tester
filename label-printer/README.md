## Label printer API

Python web api to run together with 18650 tester app

### Install

```
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

> Do not use `sudo python -m make_label` or install packages system-wide on Debian/Ubuntu. If you need to install outside a virtual environment, use `python3 -m pip install --break-system-packages -r requirements.txt` instead.

### Start

```sh
python3 label_api.py
```

### Run label script

```sh
python3 -m make_label
```

## Fix access to printer

1. Find vendor/product IDs:

```sh
lsusb
```

Eg. result:
Bus 001 Device 003: ID 04f9:2042 Brother Industries, Ltd QL-700 Label Printer

2. Create a udev rule (replace VENDOR and PRODUCT with the IDs from step 1):

Replace VENDOR with 04f9 and PRODUCT with 2042

```sh
sudo tee /etc/udev/rules.d/99-brother-ql.rules > /dev/null <<EOF
# Brother QL printer
ATTRS{idVendor}=="VENDOR", ATTRS{idProduct}=="PRODUCT", MODE="0666", TAG+="uaccess"
EOF
```

3. Reload udev rules and replug the printer:

```sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Then unplug and replug the USB printer

4. Add pi to the plugdev group (if using GROUP="plugdev"):

```sh
sudo usermod -aG plugdev pi
# then log out and back in, or reboot:
sudo reboot
```

If you still get permission errors, run ls -l /dev/bus/usb/$(printf "%03d" 

## Install Monaco font

```sh
mkdir -p ~/.local/share/fonts
cp fonts/Monaco.ttf ~/.local/share/fonts/
fc-cache -f -v
```

Check that it is installed:

```sh
fc-list | grep Monaco
```

## Install as service on Linux

1. Create ini-file:

```sh
sudo nano /etc/systemd/system/label-printer.service
```

Content:

```ini
[Unit]
Description=18650 Label Printer API
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/repos/18650-tester/label-printer
ExecStart=/home/pi/repos/18650-tester/label-printer/.venv/bin/python /home/pi/repos/18650-tester/label-printer/label_api.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

2. Start it:

```sh
sudo systemctl daemon-reload
sudo systemctl enable label-printer.service
sudo systemctl start label-printer.service
```

3. Check status:

```sh
systemctl status label-printer.service
```

4. View logs:

```sh
journalctl -u label-printer.service -f
```
