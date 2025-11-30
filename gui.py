import re
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
from collections import defaultdict
from datetime import datetime
from copy import deepcopy

# Models
class Product:
    def __init__(self, barcode, name, price, stock, expiry, category):
        self.barcode = barcode
        self.name = name
        self.price = float(price)
        self.stock = int(stock)
        self.expiry = expiry
        self.category = category

class Customer:
    def __init__(self, cid, name, ctype='walk-in'):
        self.id = cid
        self.name = name
        self.type = ctype  # 'walk-in', 'online', 'special-needs'
        self.cart = ShoppingCart()

class ShoppingCart:
    def __init__(self):
        self.items = {}  # barcode -> qty
        self.history = []
    
    def add(self, barcode, qty):
        self.history.append(('add', barcode, qty))
        self.items[barcode] = self.items.get(barcode, 0) + qty
    
    def remove(self, barcode, qty):
        if barcode in self.items:
            self.history.append(('remove', barcode, qty))
            self.items[barcode] -= qty
            if self.items[barcode] <= 0:
                del self.items[barcode]
    
    def undo(self, catalog):
        if not self.history:
            return False
        op, bc, qty = self.history.pop()
        if op == 'add':
            self.items[bc] -= qty
            if self.items[bc] <= 0:
                del self.items[bc]
            if bc in catalog:
                catalog[bc].stock += qty
        else:  # remove
            self.items[bc] = self.items.get(bc, 0) + qty
            if bc in catalog:
                catalog[bc].stock -= qty
        return True
    
    def clear(self):
        self.items.clear()
        self.history.clear()
    
    def total(self, catalog):
        tot = 0.0
        for bc, q in self.items.items():
            p = catalog.get(bc)
            if p: tot += p.price * q
        return tot
    
    def items_list(self):
        return list(self.items.items())

class SaleRecord:
    def __init__(self, sale_id, customer_id, items, total, is_online=False):
        self.sale_id = sale_id
        self.customer_id = customer_id
        self.items = list(items)
        self.total = total
        self.is_online = is_online
        self.timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

class Cashier:
    def __init__(self, cid):
        self.id = cid
        self.queue = []
        self.undo_stack = []

class App:
    def __init__(self, root):
        self.root = root
        root.title('Supermarket System GUI')
        root.geometry('1200x700')
        icon = tk.PhotoImage(file='supermarket.png')
        root.iconphoto(False, icon)
        # Set red theme for title bar and main window
        root.configure(bg="#059E94")
        style = ttk.Style()
        style.theme_use('clam')
        style.configure('TNotebook', background='#059E94', borderwidth=0)
        style.configure('TFrame', background='#F0F0F0')
        style.configure('TLabel', background='#F0F0F0')
        style.map('TNotebook.Tab', background=[('selected', '#059E94')], foreground=[('selected', 'white')])

        self.catalog = {}
        self.customers = {}
        self.cashiers = [Cashier(f"CASH{i+1}") for i in range(3)]
        self.special_cashier = Cashier("SPECIAL")
        self.online_queue = []
        self.sales = []
        self.next_sale = 1
        
        # Valid coupons: code -> discount_percentage (e.g., "SAVE10" -> 10)
        self.valid_coupons = {
            "SAVE10": 10,
            "SAVE15": 15,
            "SAVE20": 20,
            "LOVEEGYPT": 5,
            "VIP25": 25
        }
        self.used_coupons = set()  # Track used coupon codes

        self.load_products_from_system_h()

        # Create red header bar
        header = tk.Frame(root, bg='#FF3333', height=60)
        header.pack(fill='x', padx=0, pady=0)
        header.pack_propagate(False)
        title_label = tk.Label(header, text='🛒 Supermarket Management System', 
                               font=('Arial', 18, 'bold'), 
                               bg='#FF3333', fg='white', padx=20, pady=10)
        title_label.pack(side='left', padx=20)

        # Create main notebook
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(expand=True, fill='both', padx=8, pady=8)

        # Tab 1: Products & Browse
        self.tab_browse = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_browse, text='Browse Products')
        self.setup_browse_tab()

        # Tab 2: Customers
        self.tab_customers = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_customers, text='Customers')
        self.setup_customers_tab()

        # Tab 3: Cart & Checkout
        self.tab_cart = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_cart, text='Cart & Checkout')
        self.setup_cart_tab()

        # Tab 4: Queues
        self.tab_queues = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_queues, text='Cashier Queues')
        self.setup_queues_tab()

        # Tab 5: Sales Report
        self.tab_sales = ttk.Frame(self.notebook)
        self.notebook.add(self.tab_sales, text='Sales Report')
        self.setup_sales_tab()

    def load_products_from_system_h(self):
        try:
            with open('system.h', 'r', encoding='utf-8') as f:
                content = f.read()
        except Exception as e:
            messagebox.showerror('Error', f'Cannot open system.h: {e}')
            return
        pattern = re.compile(r'inventory\.add_product\(Product\("([^"]+)"\s*,\s*"([^"]+)"\s*,\s*([0-9]+(?:\.[0-9]+)?)\s*,\s*([0-9]+)\s*,\s*"([^"]*)"\s*,\s*"?([^"\)]*)"?\)\)', re.M)
        for m in pattern.finditer(content):
            bc, name, price, stock, expiry, cat = m.groups()
            prod = Product(bc.strip(), name.strip(), float(price), int(stock), expiry.strip(), cat.strip())
            self.catalog[bc] = prod

    # ===== BROWSE TAB =====
    def setup_browse_tab(self):
        left = ttk.Frame(self.tab_browse, padding=8)
        left.pack(side='left', fill='both', expand=True)
        ttk.Label(left, text='Products (Click to select)', font=('Arial', 11, 'bold')).pack()
        
        # Sorting options
        sort_frame = ttk.Frame(left)
        sort_frame.pack(fill='x', pady=6)
        ttk.Label(sort_frame, text='Sort by:').pack(side='left', padx=4)
        ttk.Button(sort_frame, text='Barcode', command=lambda: self.refresh_product_list('barcode')).pack(side='left', padx=2)
        ttk.Button(sort_frame, text='Price (Low→High)', command=lambda: self.refresh_product_list('price_asc')).pack(side='left', padx=2)
        ttk.Button(sort_frame, text='Price (High→Low)', command=lambda: self.refresh_product_list('price_desc')).pack(side='left', padx=2)
        ttk.Button(sort_frame, text='Category', command=lambda: self.refresh_product_list('category')).pack(side='left', padx=2)
        
        self.prod_list = tk.Listbox(left, width=80, height=20)
        self.prod_list.pack(side='left', fill='both', expand=True)
        scroll = ttk.Scrollbar(left, orient='vertical', command=self.prod_list.yview)
        scroll.pack(side='right', fill='y')
        self.prod_list.config(yscrollcommand=scroll.set)
        self.refresh_product_list()

        right = ttk.Frame(self.tab_browse, padding=8)
        right.pack(side='right', fill='both')
        ttk.Label(right, text='Quick Actions', font=('Arial', 11, 'bold')).pack()
        ttk.Button(right, text='Refresh Products', command=self.refresh_product_list).pack(pady=4, fill='x')
        ttk.Button(right, text='Product Details', command=self.show_product_details).pack(pady=4, fill='x')

    def refresh_product_list(self, sort_by='barcode'):
        self.prod_list.delete(0, tk.END)
        products = list(self.catalog.values())
        
        # Sort products
        if sort_by == 'price_asc':
            products.sort(key=lambda p: p.price)
        elif sort_by == 'price_desc':
            products.sort(key=lambda p: p.price, reverse=True)
        elif sort_by == 'category':
            products.sort(key=lambda p: (p.category, p.name))
        else:  # barcode
            products.sort(key=lambda p: p.barcode)
        
        for p in products:
            self.prod_list.insert(tk.END, f"{p.barcode} | {p.name} | LE{p.price:.2f} | stock: {p.stock} | {p.category}")

    def show_product_details(self):
        sel = self.prod_list.curselection()
        if not sel:
            messagebox.showinfo('Info', 'Select a product')
            return
        line = self.prod_list.get(sel[0])
        barcode = line.split('|')[0].strip()
        p = self.catalog.get(barcode)
        if p:
            messagebox.showinfo('Product Details', f"Barcode: {p.barcode}\nName: {p.name}\nPrice: LE{p.price:.2f}\nStock: {p.stock}\nExpiry: {p.expiry}\nCategory: {p.category}")

    # ===== CUSTOMERS TAB =====
    def setup_customers_tab(self):
        top = ttk.Frame(self.tab_customers, padding=8)
        top.pack(fill='x')
        ttk.Label(top, text='Add Customer', font=('Arial', 11, 'bold')).pack()
        f = ttk.Frame(top)
        f.pack(fill='x', pady=6)
        ttk.Label(f, text='ID:').pack(side='left', padx=4)
        self.cust_id_var = tk.StringVar()
        ttk.Entry(f, textvariable=self.cust_id_var, width=12).pack(side='left', padx=4)
        ttk.Label(f, text='Name:').pack(side='left', padx=4)
        self.cust_name_var = tk.StringVar()
        ttk.Entry(f, textvariable=self.cust_name_var, width=20).pack(side='left', padx=4)
        ttk.Label(f, text='Type:').pack(side='left', padx=4)
        self.cust_type_var = tk.StringVar(value='walk-in')
        ttk.Combobox(f, textvariable=self.cust_type_var, values=['walk-in', 'online', 'special-needs'], width=12, state='readonly').pack(side='left', padx=4)
        ttk.Button(f, text='Add Customer', command=self.add_customer).pack(side='left', padx=4)

        mid = ttk.Frame(self.tab_customers, padding=8)
        mid.pack(fill='both', expand=True)
        ttk.Label(mid, text='Customers', font=('Arial', 11, 'bold')).pack()
        self.cust_list = tk.Listbox(mid, width=80, height=15)
        self.cust_list.pack(side='left', fill='both', expand=True)
        scroll = ttk.Scrollbar(mid, orient='vertical', command=self.cust_list.yview)
        scroll.pack(side='right', fill='y')
        self.cust_list.config(yscrollcommand=scroll.set)

    def add_customer(self):
        cid = self.cust_id_var.get()
        name = self.cust_name_var.get()
        ctype = self.cust_type_var.get()
        if not cid or not name:
            messagebox.showinfo('Info', 'Enter ID and Name')
            return
        if cid in self.customers:
            messagebox.showerror('Error', 'Customer already exists')
            return
        self.customers[cid] = Customer(cid, name, ctype)
        self.refresh_customers_list()
        self.cust_id_var.set('')
        self.cust_name_var.set('')

    def refresh_customers_list(self):
        self.cust_list.delete(0, tk.END)
        for cid, c in self.customers.items():
            self.cust_list.insert(tk.END, f"{cid} | {c.name} | {c.type} | Cart: {len(c.cart.items)} items")

    # ===== CART TAB =====
    def setup_cart_tab(self):
        top = ttk.Frame(self.tab_cart, padding=8)
        top.pack(fill='x')
        ttk.Label(top, text='Select Customer & Manage Cart', font=('Arial', 11, 'bold')).pack()
        f = ttk.Frame(top)
        f.pack(fill='x', pady=6)
        ttk.Label(f, text='Customer:').pack(side='left', padx=4)
        self.sel_cust_var = tk.StringVar()
        self.sel_cust_combo = ttk.Combobox(f, textvariable=self.sel_cust_var, values=[], width=20, state='readonly')
        self.sel_cust_combo.pack(side='left', padx=4)
        ttk.Button(f, text='Refresh customers', command=self.refresh_cust_combo).pack(side='left', padx=4)

        mid = ttk.Frame(self.tab_cart, padding=8)
        mid.pack(fill='both', expand=True)
        ttk.Label(mid, text='Add to Cart', font=('Arial', 11, 'bold')).pack()
        f2 = ttk.Frame(mid)
        f2.pack(fill='x', pady=6)
        ttk.Label(f2, text='Product Barcode:').pack(side='left', padx=4)
        self.add_barcode_var = tk.StringVar()
        ttk.Entry(f2, textvariable=self.add_barcode_var, width=12).pack(side='left', padx=4)
        ttk.Label(f2, text='Qty:').pack(side='left', padx=4)
        self.add_qty_var = tk.IntVar(value=1)
        ttk.Entry(f2, textvariable=self.add_qty_var, width=6).pack(side='left', padx=4)
        ttk.Button(f2, text='Add', command=self.add_to_cart).pack(side='left', padx=4)

        ttk.Label(mid, text='Shopping Cart', font=('Arial', 11, 'bold')).pack()
        self.cart_list = tk.Listbox(mid, width=80, height=10)
        self.cart_list.pack(side='left', fill='both', expand=True)
        scroll = ttk.Scrollbar(mid, orient='vertical', command=self.cart_list.yview)
        scroll.pack(side='right', fill='y')
        self.cart_list.config(yscrollcommand=scroll.set)

        bottom = ttk.Frame(self.tab_cart, padding=8)
        bottom.pack(fill='x')
        ttk.Button(bottom, text='Remove selected from cart', command=self.remove_from_cart).pack(side='left', padx=4)
        ttk.Button(bottom, text='Undo last action', command=self.undo_cart).pack(side='left', padx=4)
        ttk.Button(bottom, text='Clear cart', command=self.clear_cart).pack(side='left', padx=4)
        ttk.Button(bottom, text='View cart total', command=self.view_cart_total).pack(side='left', padx=4)

        # Coupon section
        coupon_frame = ttk.Frame(self.tab_cart, padding=8)
        coupon_frame.pack(fill='x')
        ttk.Label(coupon_frame, text='Coupons:', font=('Arial', 10, 'bold')).pack(side='left', padx=4)
        ttk.Label(coupon_frame, text='Code:').pack(side='left', padx=2)
        self.coupon_var = tk.StringVar()
        ttk.Entry(coupon_frame, textvariable=self.coupon_var, width=15).pack(side='left', padx=2)
        ttk.Button(coupon_frame, text='Valid Coupons', command=self.show_valid_coupons).pack(side='left', padx=4)

    def refresh_cust_combo(self):
        self.sel_cust_combo['values'] = [f"{cid} ({c.name})" for cid, c in self.customers.items()]

    def get_selected_customer(self):
        sel = self.sel_cust_var.get()
        if not sel or '(' not in sel:
            messagebox.showinfo('Info', 'Select a customer')
            return None
        cid = sel.split('(')[0].strip()
        return self.customers.get(cid)

    def add_to_cart(self):
        c = self.get_selected_customer()
        if not c: return
        bc = self.add_barcode_var.get()
        qty = self.add_qty_var.get()
        if not bc or qty <= 0:
            messagebox.showinfo('Info', 'Enter valid barcode and qty')
            return
        p = self.catalog.get(bc)
        if not p:
            messagebox.showerror('Error', f'Product {bc} not found')
            return
        if p.stock < qty:
            messagebox.showerror('Error', f'Not enough stock (available {p.stock})')
            return
        p.stock -= qty
        c.cart.add(bc, qty)
        self.refresh_product_list()
        self.refresh_cart_list()
        self.add_barcode_var.set('')

    def refresh_cart_list(self):
        c = self.get_selected_customer()
        if not c: return
        self.cart_list.delete(0, tk.END)
        for bc, q in c.cart.items_list():
            p = self.catalog.get(bc)
            name = p.name if p else '<unknown>'
            price = p.price if p else 0.0
            self.cart_list.insert(tk.END, f"{bc} | {name} | qty: {q} | unit: LE{price:.2f}")

    def remove_from_cart(self):
        c = self.get_selected_customer()
        if not c: return
        sel = self.cart_list.curselection()
        if not sel:
            messagebox.showinfo('Info', 'Select item')
            return
        line = self.cart_list.get(sel[0])
        bc = line.split('|')[0].strip()
        if bc in c.cart.items:
            qty = c.cart.items[bc]
            c.cart.remove(bc, qty)
            if bc in self.catalog:
                self.catalog[bc].stock += qty
        self.refresh_product_list()
        self.refresh_cart_list()

    def undo_cart(self):
        c = self.get_selected_customer()
        if not c: return
        if c.cart.undo(self.catalog):
            messagebox.showinfo('Info', 'Undid last action')
        else:
            messagebox.showinfo('Info', 'Nothing to undo')
        self.refresh_product_list()
        self.refresh_cart_list()

    def clear_cart(self):
        c = self.get_selected_customer()
        if not c: return
        for bc, q in list(c.cart.items_list()):
            if bc in self.catalog:
                self.catalog[bc].stock += q
        c.cart.clear()
        self.refresh_product_list()
        self.refresh_cart_list()

    def view_cart_total(self):
        c = self.get_selected_customer()
        if not c: return
        tot = c.cart.total(self.catalog)
        messagebox.showinfo('Cart Total', f"Current total: LE {tot:.2f}")

    # ===== QUEUES TAB =====
    def setup_queues_tab(self):
        ttk.Label(self.tab_queues, text='Enqueue & Checkout', font=('Arial', 11, 'bold')).pack(padx=8, pady=8)

        # Enqueue section
        f = ttk.Frame(self.tab_queues, padding=8)
        f.pack(fill='x')
        ttk.Label(f, text='Enqueue Customer:').pack(side='left', padx=4)
        self.enq_cust_var = tk.StringVar()
        self.enq_cust_combo = ttk.Combobox(f, textvariable=self.enq_cust_var, values=[], width=20, state='readonly')
        self.enq_cust_combo.pack(side='left', padx=4)
        ttk.Button(f, text='Enqueue', command=self.enqueue_customer).pack(side='left', padx=4)
        ttk.Button(f, text='Refresh', command=lambda: self.enq_cust_combo.config(values=[f"{cid} ({c.name})" for cid, c in self.customers.items()])).pack(side='left', padx=4)

        # Checkout section
        mid = ttk.Frame(self.tab_queues, padding=8)
        mid.pack(fill='x')
        ttk.Label(mid, text='Process Checkout from:', font=('Arial', 10, 'bold')).pack(side='left', padx=4)
        self.cashier_var = tk.StringVar(value='CASH1')
        ttk.Combobox(mid, textvariable=self.cashier_var, values=['CASH1', 'CASH2', 'CASH3', 'SPECIAL'], width=12, state='readonly').pack(side='left', padx=4)
        ttk.Button(mid, text='Checkout', command=self.process_checkout).pack(side='left', padx=4)
        ttk.Button(mid, text='Undo last bill', command=self.undo_bill).pack(side='left', padx=4)

        # Online orders section
        online_sec = ttk.Frame(self.tab_queues, padding=8)
        online_sec.pack(fill='x')
        ttk.Label(online_sec, text='Process Online Order:', font=('Arial', 10, 'bold')).pack(side='left', padx=4)
        ttk.Button(online_sec, text='Process Next Online Order', command=self.process_online_order).pack(side='left', padx=4)

        # Queue status
        bottom = ttk.Frame(self.tab_queues, padding=8)
        bottom.pack(fill='both', expand=True)
        ttk.Label(bottom, text='Queue Status', font=('Arial', 10, 'bold')).pack()
        self.queue_text = tk.Text(bottom, height=15, width=100)
        self.queue_text.pack(fill='both', expand=True)

        ttk.Button(self.tab_queues, text='Refresh Queue Status', command=self.refresh_queue_status).pack(pady=8)

    def enqueue_customer(self):
        sel = self.enq_cust_var.get()
        if not sel or '(' not in sel:
            messagebox.showinfo('Info', 'Select a customer')
            return
        cid = sel.split('(')[0].strip()
        c = self.customers.get(cid)
        if not c: return
        if c.type == 'special-needs':
            self.special_cashier.queue.append(c)
            messagebox.showinfo('Info', f"{c.name} enqueued to SPECIAL cashier")
        elif c.type == 'online':
            self.online_queue.append(c)
            messagebox.showinfo('Info', f"{c.name} added to online queue")
        else:  # walk-in
            self.cashiers[0].queue.append(c)  # use first cashier for simplicity
            messagebox.showinfo('Info', f"{c.name} enqueued to CASH1")
        self.refresh_queue_status()

    def process_checkout(self):
        cashier_id = self.cashier_var.get()
        if cashier_id == 'SPECIAL':
            cashier = self.special_cashier
        else:
            idx = int(cashier_id[-1]) - 1
            cashier = self.cashiers[idx]

        if not cashier.queue:
            messagebox.showinfo('Info', 'Queue is empty')
            return

        c = cashier.queue.pop(0)
        if not c.cart.items:
            messagebox.showinfo('Info', f"{c.name} has empty cart")
            return

        # Apply discounts
        tot = c.cart.total(self.catalog)
        coupon_discount = 0.0
        
        # Apply coupon if provided
        coupon_code = self.coupon_var.get().strip().upper()
        if coupon_code:
            if coupon_code in self.valid_coupons and coupon_code not in self.used_coupons:
                coupon_discount = tot * (self.valid_coupons[coupon_code] / 100.0)
                tot -= coupon_discount
                self.used_coupons.add(coupon_code)
                messagebox.showinfo('Coupon Applied', f'Coupon {coupon_code} applied: -LE {coupon_discount:.2f}')
            elif coupon_code in self.used_coupons:
                messagebox.showwarning('Coupon', f'Coupon {coupon_code} already used')
            else:
                messagebox.showwarning('Coupon', f'Invalid coupon: {coupon_code}')
        
        # Apply special-needs 10% discount
        if c.type == 'special-needs':
            discount = tot * 0.10
            tot -= discount
        
        # Apply bulk discount 5% for >= 1000
        if tot >= 1000.0:
            tot = tot * 0.95

        sid = f"S{self.next_sale}"
        self.next_sale += 1
        rec = SaleRecord(sid, c.id, c.cart.items_list(), tot, c.type == 'online')
        self.sales.append(rec)
        cashier.undo_stack.append(rec)

        messagebox.showinfo('Checkout', f'Sale {sid} complete.\nCustomer: {c.name}\nTotal: LE {tot:.2f}')
        c.cart.clear()
        self.coupon_var.set('')
        self.refresh_queue_status()

    def undo_bill(self):
        cashier_id = self.cashier_var.get()
        if cashier_id == 'SPECIAL':
            cashier = self.special_cashier
        else:
            idx = int(cashier_id[-1]) - 1
            cashier = self.cashiers[idx]

        if not cashier.undo_stack:
            messagebox.showinfo('Info', 'No bills to undo')
            return

        rec = cashier.undo_stack.pop()
        for bc, q in rec.items:
            if bc in self.catalog:
                self.catalog[bc].stock += q
        self.sales = [s for s in self.sales if s.sale_id != rec.sale_id]

        messagebox.showinfo('Undo', f'Undid sale {rec.sale_id} and restored stock')
        self.refresh_queue_status()

    def refresh_queue_status(self):
        self.queue_text.delete(1.0, tk.END)
        text = "=== CASHIER QUEUES ===\n"
        for i, cs in enumerate(self.cashiers):
            text += f"\n{cs.id} ({len(cs.queue)} customers):\n"
            for c in cs.queue:
                text += f"  - {c.id} | {c.name} | Cart items: {len(c.cart.items)}\n"
        text += f"\nSPECIAL ({len(self.special_cashier.queue)} customers):\n"
        for c in self.special_cashier.queue:
            text += f"  - {c.id} | {c.name} | Cart items: {len(c.cart.items)}\n"
        text += f"\nONLINE ({len(self.online_queue)} orders):\n"
        for c in self.online_queue:
            text += f"  - {c.id} | {c.name} | Cart items: {len(c.cart.items)}\n"
        self.queue_text.insert(1.0, text)

    def process_online_order(self):
        if not self.online_queue:
            messagebox.showinfo('Info', 'No online orders in queue')
            return

        c = self.online_queue.pop(0)
        if not c.cart.items:
            messagebox.showinfo('Info', f"{c.name} has empty cart")
            return

        # Apply discounts (no special-needs discount for online customers)
        tot = c.cart.total(self.catalog)
        coupon_discount = 0.0
        
        # Apply coupon if provided
        coupon_code = self.coupon_var.get().strip().upper()
        if coupon_code:
            if coupon_code in self.valid_coupons and coupon_code not in self.used_coupons:
                coupon_discount = tot * (self.valid_coupons[coupon_code] / 100.0)
                tot -= coupon_discount
                self.used_coupons.add(coupon_code)
                messagebox.showinfo('Coupon Applied', f'Coupon {coupon_code} applied: -LE {coupon_discount:.2f}')
            elif coupon_code in self.used_coupons:
                messagebox.showwarning('Coupon', f'Coupon {coupon_code} already used')
            else:
                messagebox.showwarning('Coupon', f'Invalid coupon: {coupon_code}')
        
        if tot >= 1000.0:
            tot = tot * 0.95

        sid = f"S{self.next_sale}"
        self.next_sale += 1
        rec = SaleRecord(sid, c.id, c.cart.items_list(), tot, is_online=True)
        self.sales.append(rec)
        if self.cashiers:
            self.cashiers[0].undo_stack.append(rec)

        messagebox.showinfo('Online Order Processed', f'Sale {sid} complete.\nCustomer: {c.name}\nTotal: LE {tot:.2f}')
        c.cart.clear()
        self.coupon_var.set('')
        self.refresh_queue_status()

    # ===== SALES TAB =====
    def setup_sales_tab(self):
        top = ttk.Frame(self.tab_sales, padding=8)
        top.pack(fill='x')
        ttk.Label(top, text='Sales Report', font=('Arial', 11, 'bold')).pack(side='left')
        ttk.Button(top, text='Refresh Report', command=self.refresh_sales_report).pack(side='left', padx=4)
        ttk.Button(top, text='Export to CSV', command=self.export_sales_csv).pack(side='left', padx=4)

        mid = ttk.Frame(self.tab_sales, padding=8)
        mid.pack(fill='both', expand=True)
        ttk.Label(mid, text='Sales Records', font=('Arial', 10, 'bold')).pack()
        self.sales_records_text = tk.Text(mid, height=8, width=120)
        self.sales_records_text.pack(fill='both', expand=True)

        ttk.Label(mid, text='Top Sold Products', font=('Arial', 10, 'bold')).pack()
        self.sales_top_text = tk.Text(mid, height=10, width=120)
        self.sales_top_text.pack(fill='both', expand=True)

    def refresh_sales_report(self):
        # Sales records
        self.sales_records_text.delete(1.0, tk.END)
        text = "=== SALES RECORDS ===\n"
        for s in self.sales:
            text += f"{s.sale_id} | Cust: {s.customer_id} | {'Online' if s.is_online else 'Walk-in'} | LE {s.total:.2f} | {s.timestamp}\n"
            for bc, q in s.items:
                p = self.catalog.get(bc)
                name = p.name if p else '<unknown>'
                text += f"   - {bc} ({name}) x{q}\n"
        self.sales_records_text.insert(1.0, text)

        # Top sold products
        self.sales_top_text.delete(1.0, tk.END)
        tally = defaultdict(int)
        for s in self.sales:
            for bc, q in s.items:
                tally[bc] += q

        rows = []
        for bc, q in tally.items():
            p = self.catalog.get(bc)
            name = p.name if p else '<unknown>'
            revenue = (p.price if p else 0.0) * q
            rows.append((bc, name, q, revenue))
        rows.sort(key=lambda x: x[2], reverse=True)

        top_text = "=== TOP SOLD PRODUCTS ===\n"
        for i, (bc, name, q, rev) in enumerate(rows[:10], 1):
            top_text += f"{i}. {bc} | {name} | qty: {q} | revenue: LE {rev:.2f}\n"
        self.sales_top_text.insert(1.0, top_text)

    def export_sales_csv(self):
        if not self.sales:
            messagebox.showinfo('Info', 'No sales to export')
            return
        try:
            with open('sales_export.csv', 'w', encoding='utf-8') as f:
                f.write('Sale ID,Customer ID,Type,Total (LE),Timestamp,Items\n')
                for s in self.sales:
                    items_str = ';'.join([f"{bc}x{q}" for bc, q in s.items])
                    f.write(f'{s.sale_id},{s.customer_id},{"Online" if s.is_online else "Walk-in"},{s.total:.2f},{s.timestamp},{items_str}\n')
            messagebox.showinfo('Success', 'Exported to sales_export.csv')
        except Exception as e:
            messagebox.showerror('Error', f'Export failed: {e}')

    def show_valid_coupons(self):
        msg = "Valid Coupons (each can be used once):\n\n"
        for code, discount in self.valid_coupons.items():
            status = "(USED)" if code in self.used_coupons else "(Available)"
            msg += f"• {code}: {discount}% off {status}\n"
        messagebox.showinfo('Available Coupons', msg)

if __name__ == '__main__':
    root = tk.Tk()
    app = App(root)
    root.mainloop()
