# Supermarket OOP + Online Demo

A small C++ (C++17) supermarket demo with walk-in and online customers, shopping carts, checkouts, and a simple interactive console.

**Files**
- `main.cpp` — thin entrypoint that includes `system.h` and starts the interactive console.
**Build (Windows PowerShell / MinGW)**

Open PowerShell in the project folder and run:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -o main.exe main.cpp
```

Then run the program:

```powershell
.\main.exe
```

Or run an automated script (example provided):

```powershell
Get-Content commands_all.txt | .\main.exe
```


## Supermarket Management — C++ Console + Python Tkinter GUI

This repository contains two complementary implementations of a supermarket demo:

- A C++ console application (header-first code: `system.h`, `cart.h`, `sales.h`, etc.) that implements inventory, customers (walk-in and online), carts, cashier queues, undo, and sales reporting.
- A standalone Python/Tkinter GUI (`gui.py`) that re-implements the same supermarket features in a graphical interface and reads the seeded product list from `system.h`.

This README focuses on how to run and use both the console and the GUI, explains the main features, and gives notes about the architecture and extension points.

**Files (high level)**
- `main.cpp` — thin C++ entry point.
- `system.h`, `inventory.h`, `product.h`, `cart.h`, `sales.h`, `customer.h`, `bst.h`, `utils.h` — main C++ implementation (header-heavy).
- `gui.py` — Python Tkinter GUI application that mirrors the C++ features (customers, cart, checkout, cashier queues, sales reporting, coupons).

**Highlights / Features**
- Inventory with seeded products (parsed from `system.h` when launching `gui.py`).
- Customer types: walk-in, online, special-needs.
- Shopping cart with add/remove and undo functionality.
- Multiple cashier queues (3 regular + 1 special cashier for special-needs customers).
- Online orders queue and processing.
- Discounts and coupons:
	- Coupons: `SAVE10` (10%), `SAVE15` (15%), `SAVE20` (20%), `LOVEEGYPT` (5%), `VIP25` (25%) — each coupon is one-time use per session in the GUI.
	- Special-needs customers receive an automatic 10% discount (applied after coupons).
	- Bulk discount: additional 5% if the final total after coupon and special discount is >= LE1000.
- Sales recording and reporting (includes CSV export in the GUI).

## Running the C++ Console (optional)
Requires a C++17 compiler (MinGW/g++ on Windows is used in examples).

Open PowerShell in the project root and run:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -o main.exe main.cpp
.\main.exe
```

The console app provides an interactive menu. The code is header-heavy for didactic reasons; if you prefer, I can help split implementations into `.cpp` files.

## Running the Python GUI (recommended for easier use)
Requires Python 3.x with Tkinter (normally included with most Python installations on Windows).

Run from PowerShell in the project root:

```powershell
python gui.py
```

What the GUI provides (key screens):
- Browse Products: sort by barcode, price, or category; view details.
- Customers: add walk-in / online / special-needs customers.
- Cart & Checkout: manage cart, apply coupon codes, undo cart actions.
- Cashier Queues: enqueue customers (auto-routes special-needs to the special cashier), process checkouts, undo last bill, process online orders.
- Sales Report: view sales, top-sold products, export CSV.

## How discounts are applied (GUI)
1. Coupon (if entered and valid) is applied first.
2. If customer is `special-needs`, a 10% discount is applied next.
3. If the remaining total is >= LE1000, an additional 5% bulk discount is applied.

## Adding an icon to the GUI header
The GUI header already includes a shopping-cart emoji. To use a real image icon:

1. Place an image file (e.g., `icon.png`) in the project folder.
2. Update `gui.py` header code to load and display the image using `tk.PhotoImage` (recommended: a 48x48 PNG). Example snippet:

```python
from PIL import Image, ImageTk  # optional, for non-PNG formats or resizing

img = Image.open('icon.png').resize((48,48))
icon = ImageTk.PhotoImage(img)
icon_label = tk.Label(header, image=icon, bg='#FF3333')
icon_label.image = icon
icon_label.pack(side='left', padx=8)
```

If you don't have Pillow installed and the image is PNG, you can use Tkinter's built-in `tk.PhotoImage` directly (no extra dependency):

```python
icon = tk.PhotoImage(file='icon.png')
icon_label = tk.Label(header, image=icon, bg='#FF3333')
icon_label.image = icon
icon_label.pack(side='left', padx=8)
```

Note: If you want, I can add this image-loading code to `gui.py` and include a small fallback (emoji) when the image is missing.

## Architecture & Notes
- The Python GUI is standalone: it does not compile or call the C++ code. It parses `system.h` to seed products, then runs purely in Python memory.
- The C++ implementation is a separate, console-oriented reference implementation.
- Persistence: inventory and sales are in-memory in the GUI. If you want session persistence, I can add JSON save/load.

## Development & Next Steps (suggested)
- Persist inventory and sales to `data/` JSON files between sessions.
- Add product search (full-text) and filters in the GUI.
- Add per-item recording in the C++ checkout flow so sales reports include per-item tallies.
- Add a small icon image and include it in the header (I can do that for you).

## License & Credits
This demo is provided for educational purposes. Feel free to modify and extend it.

---
If you'd like, I can now:
- Add image icon support to `gui.py` and include a sample icon file;
- Add session persistence (JSON) for inventory and sales;
- Split the C++ headers into `.cpp` files for a cleaner build.

Tell me which you'd like next and I will implement it.