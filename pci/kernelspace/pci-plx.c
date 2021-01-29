#include "device.h"
#include "camera.h"
#include "registers-plx-pci.h"
#include "../userspace/constants.h"
#include <linux/delay.h>
#include <linux/uaccess.h>

int init_hardware(struct dev_struct *dev, struct board_vars *vars) {
  u8 reg_b;
  u32 reg_l;

  writeb(0x23, dev->pci_camera + CAMERA_CTRLA); // POS SLOPE | IFC=1 | V_ON
  writeb(0x40, dev->pci_camera + CAMERA_CTRLB); // F_DP double pulse 16bit
  reg_b = readb(dev->pci_camera + CAMERA_CTRLC);
  reg_b |= 0x08; /* burst flag, ND-freq, 33MHz data rate with fifo */
  writeb(reg_b, dev->pci_camera + CAMERA_CTRLC);
  writel(dev->n_pixels * 2 | (vars->pclk << 16) | (vars->xckdelay << 19),
         dev->pci_camera + CAMERA_PIXREG);

  /* DELAY register */
  reg_l = ~readl(dev->pci_camera);
  if (reg_l > 0x24) writel(0, dev->pci_camera + CAMERA_EC);
  if (reg_l > 0x34) writel(0, dev->pci_camera + CAMERA_DELAYREG);

  /* reset TOREG */
  writeb(0, dev->pci_camera + CAMERA_TOR0);

  reg_b = readb(dev->pci_camera + CAMERA_TOR3);
  reg_b |= 0x02; /* PDA = false */
  reg_b &= ~0x01; /* FFT = true */
  writeb(reg_b, dev->pci_camera + CAMERA_TOR3);

  /* VCLK register */
  writel(dev->lines * 2, dev->pci_camera + CAMERA_VCLKCTRL);
  writeb(dev->vfreq, dev->pci_camera + CAMERA_VCLKFREQ);

  /* reset FIFO */
  reg_b = readb(dev->pci_camera + CAMERA_FREQREG);
  reg_b |= 0x80;
  writeb(reg_b, dev->pci_camera + CAMERA_FREQREG);
  reg_b &= ~0x80;
  writeb(reg_b, dev->pci_camera + CAMERA_FREQREG);

  /* external trigger */
  reg_b = readb(dev->pci_camera + CAMERA_XCKMSB);
  reg_b |= 0x80;
  writeb(reg_b, dev->pci_camera + CAMERA_XCKMSB);

  return 0;
}

int clear_fifo(struct dev_struct *dev) {
  int reg_val;

  PDEBUG(D_START_STOP, "clear fifo\n");
  reg_val = readb(dev->pci_camera + CAMERA_FREQREG);
  reg_val |= B_RESET_FIFO;
  writeb(reg_val, dev->pci_camera + CAMERA_FREQREG);
  udelay(5);
  reg_val &= ~B_RESET_FIFO;
  writeb(reg_val, dev->pci_camera + CAMERA_FREQREG);

  return 0;
}

int enable_trigger(struct dev_struct *dev) {
  u8 xck_msb;

  PDEBUG(D_START_STOP, "starting camera\n");

  writel(0, dev->pci_camera + CAMERA_EC);

  xck_msb = readb(dev->pci_camera + CAMERA_XCKMSB);
  xck_msb |= B_TRIG_EXTERN | B_TIMER_RESET;
  writeb(xck_msb, dev->pci_camera + CAMERA_XCKMSB);

  return 0;
}

int disable_trigger(struct dev_struct *dev) {
  u8 xck_msb;

  xck_msb = readb(dev->pci_camera + CAMERA_XCKMSB);
  xck_msb &= ~B_TIMER_RESET;
  writeb(xck_msb, dev->pci_camera + CAMERA_XCKMSB);

  return 0;
}

int read_fifo_overflow(struct dev_struct *dev) {
  return readb(dev->pci_camera + CAMERA_FFFLAGS) & B_FIFO_OVERFLOW;
}

int write_register(u8 what, u32 where, struct dev_struct *dev) {
  writeb(what, dev->pci_camera + where);
  return 0;
}

int write_register_word(u16 what, u32 where, struct dev_struct *dev) {
  writew(what, dev->pci_camera + where);
  return 0;
}

int write_register_dword(u32 what, u32 where, struct dev_struct *dev) {
  writel(what, dev->pci_camera + where);
  return 0;
}

int write_plx(u8 what, u32 where, struct dev_struct *dev) {
  writeb(what, dev->pci_config + where);  
  return 0;
}

/******************************************************************************/
/*                                register access                             */
/******************************************************************************/

#ifdef DEBUG
# define _writeb(val, address) do { \
    if (dev->no_hardware) *(address) = val;    \
    else writeb(val, address); } while (0)
# define _writew(val, address) do { \
    if (dev->no_hardware) *(uint16_t*)(address) = val; \
    else writew(val, address); } while (0)
# define _writel(val, address) do { \
    if (dev->no_hardware) *(uint32_t*)(address) = val; \
    else writel(val, address); } while (0)
# define _readb(address) (dev->no_hardware ? *(address) : readb(address))
# define _readw(address) \
  (dev->no_hardware ? *(uint16_t*)(address) : readw(address))
# define _readl(address) \
  (dev->no_hardware ? *(uint32_t*)(address) : readl(address))
#else
# define _writeb writeb
# define _writew writew
# define _writel writel
#endif

#define DEF_BIT8(flag, bit) \
  do { if (params->flags & flag) register_val |= bit; } while (0)
#define DEF_BIT32(flag, bit) \
  do { if (params->flags & flag) reg32_val |= bit; } while (0)
#define GET_BIT8(flag, bit) do { \
    if (register_val & bit) params->flags |= flag; } while (0)
#define GET_BIT32(flag, bit) do { \
    if (reg32_val & bit) params->flags |= flag; } while (0)


int get_params(struct dev_struct *dev, struct lscpci_param *params) {
  u8 register_val = 0;
  uint32_t reg32_val;

  if (!dev) {
    printk(KERN_WARNING NAME " dev is null\n");
    return -EFAULT;
  }

  PDEBUG(D_INTERRUPT, "looking for camera address at 0x%lx, is 0x%lx\n",
         (long int) &dev->pci_camera,
         (long int) dev->pci_camera);

  PDEBUG(D_INTERRUPT, "filling buffer with zeros\n");

  memset(params, 0, sizeof(struct lscpci_param));
  if (!dev->hardware_present) {
    PDEBUG(D_INTERRUPT, " no hardware, returning empty record");
    return 0;
  }
  if (!dev->pci_camera) {
    printk(KERN_WARNING NAME " get_params no device present\n");
    return -ENODEV;
  }

  /* CtrlA */
  register_val = readb(dev->pci_camera + CAMERA_CTRLA);
  GET_BIT8(AMPLIFIER_ON, B_AMPLIFIER);
  GET_BIT8(TRIG_POS, B_TRIG_POS_SLOPE);
  GET_BIT8(TRIG_EDGE, B_TRIG_EDGE);

  /* CtrlB */
  register_val = readb(dev->pci_camera + CAMERA_CTRLB);
  GET_BIT8(NDSYM, B_NDSYM);
  GET_BIT8(DOUBLE_PULSE, B_DOUBLE_PULSE);

  /* CtrlC */
  register_val = readb(dev->pci_camera + CAMERA_CTRLC);
  GET_BIT8(OPTO1, B_OPT1);
  GET_BIT8(OPTO2, B_OPT2);
  GET_BIT8(BURST_MODE, B_BURST);

  /* XCK MSB */
  reg32_val = readl(dev->pci_camera + CAMERA_XCK32);

  params->exposure_time = reg32_val & 0x00FFFFFF;
  reg32_val >>= 24;
  if (reg32_val & B_RESOLUTION_MS) params->flags |= DIVIDER_LOW;
  if (reg32_val & B_RESOLUTION_NS) params->flags |= DIVIDER_HIGH;

  /* pixel register set at module init (-> size of dma buffers ) */

  /* FREQREG */
  register_val = readb(dev->pci_camera + CAMERA_FREQREG);
  params->pclk = register_val & FREQ_MASK;
  params->xck_delay = (register_val & DELAY_MASK) >> 3;

  /* VCLKCTRL */
  params->vertical_count = readw(dev->pci_camera + CAMERA_VCLKCTRL) >> 1;

  /* VCLKFREQ */
  params->vertical_frequency = readb(dev->pci_camera + CAMERA_VCLKFREQ);

  /* DAT */
  reg32_val = readl(dev->pci_camera + CAMERA_DAT);
  params->delay_after_trigger
    = reg32_val & 0x80000000 ? reg32_val & 0x7FFFFFFF : 0;

  /* EC */
  reg32_val = readl(dev->pci_camera + CAMERA_EC);
  params->exposure_time = reg32_val & 0x80000000 ? reg32_val & 0x7FFFFFFF : 0;

  /* TOR */
  reg32_val = readl(dev->pci_camera + CAMERA_TOR32);
  params->trigger_in_divider = reg32_val & 0x00FF;
  params->trigger_out_divider = (reg32_val & 0x00FF0000) >> 16;

  switch (reg32_val & OUT_MASK) {
  case B_DELAY_ACTIVE:      params->out_select = out_dat;    break;
  case B_FIFO_READ_ACTIVE:  params->out_select = out_ffxck;  break;
  case B_SIGNAL_TRIGGER_IN: params->out_select = out_ec;     break;
  case B_SENSOR_ACTIVE:     params->out_select = out_tin;    break;
  case B_TO_REG:            params->out_select = out_to_reg; break;
  default:                  params->out_select = out_xck;
  }

  /* DELAYREG */
  params->fifo_delay = readl(dev->pci_camera + CAMERA_DELAYREG) & 0x03FF;

  return 0;
}

int dump_registers(struct dev_struct *dev, void __user *arg) {
  int i, result;
  unsigned char lscpci_reg[NUM_CAMERA_REGISTERS];
  uint32_t plx_reg[NUM_PLX_REGISTERS];

  if (!dev->hardware_present) {
    printk(KERN_ERR NAME " can't dump what's not there\n");
    return -ENODEV;
  }

  for (i = 0; i < NUM_CAMERA_REGISTERS; i++)
    PDEBUG(D_IOCTL, "reading camera register %d\n", i);

  result = copy_to_user(arg, lscpci_reg, NUM_CAMERA_REGISTERS);
  if (result) return -EACCES;

  for (i = 4; i < NUM_PLX_REGISTERS; i += 4)
    PDEBUG(D_IOCTL, "reading plx register %d\n", i);

  result
    = copy_to_user(((u8*) arg) + NUM_CAMERA_REGISTERS,
                   plx_reg, NUM_PLX_REGISTERS * 4);
  if (result) return -EACCES;

  return result;
}
