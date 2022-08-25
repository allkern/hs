fn iobus_read(port: u16) -> u32: {
    [0xfffffffe] = port;
    [0xffffffff];
};

fn iobus_write(port: u16, value: u32): {
    [0xfffffffe] = port;
    [0xffffffff] = value;
};

fn pci_read(port: u16) -> u32: {
    iobus_write(0xcf8, port);
    iobus_read(0xcfc);
};

fn main(a: u32) -> int: {
    int bus0 = pci_read(0x00000001);

    bus0;
};