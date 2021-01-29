/*
- check for dma being done, sync for cpu, update pointers
- check fifo for valid data, set up corresponding chain and start dma
 */
void camera_poll(struct timer_list *t) {
  u8 fifo_count, something_to_read = 0, next_write, buffers_to_transfer;
  struct dev_struct *dev = from_timer(dev, t, poll_timer);

  if (dev->status & CAMERA_STOP) {
    PDEBUG(D_POLLING, "asked to stop polling\n");
    if (plx9056_dma_active(dev)) { /* set abort bit in PLX9056 */
      plx9065_abort_dma(dev);
      restart_timer(dev);
      PDEBUG(D_POLLING, "aborting DMA");
      return;
    }

    dev->status &= ~CAMERA_ACQUIRING;
    PDEBUG(D_POLLING, "stopped camera, waking up reading processed\n");
    wake_up_interruptible(&dev->readq);
    return;
  }

  if (dev->status & CAMERA_DMA_ACTIVE) {
    /* check for DMA done bit set and reset it in case */
    if (plx9056_dma_active(dev)) {
      restart_timer(dev); /* DMA busy, don't interfere */
      return;
    }

    something_to_read = 1;
    dev->status &= ~CAMERA_DMA_ACTIVE;
    
    if (dev->status & CAMERA_BUFFER_FULL) {
      /* last buffer was used, increase write pointer if there's space now */
      next_write = (dev->dma.ptrs.write_buf + 1) % dev->dma.n_buffers;
      if (next_write != dev->dma.ptrs.read_buf) {
        dev->dma.ptrs.write_buf = next_write;
        dev->status &= ~CAMERA_BUFFER_FULL;
      }
    }

    /* sync buffers to cpu previously set-up for now finished DMA */
    PDEBUG(D_POLLING, "DMA done, start syncing to cpu at buffer %d\n",
           dev->dma.ptrs.write_buf);
    while (dev->dma.ptrs.read_buf != dev->dma.ptrs.write_buf) {
      dma_sync_single_for_cpu(&dev->pci_dev->dev,
                              dev->dma.descriptors[dev->dma.ptrs.write_buf]
                              .pci_address,
                              dev->dma.buf_size, DMA_FROM_DEVICE);
      dev->dma.ptrs.write_buf
        = (dev->dma.ptrs.write_buf + 1) % dev->dma.n_buffers;
    }
    PDEBUG(D_POLLING, "end syncing to cpu at buffer %d\n",
           dev->dma.ptrs.write_buf);
  }

  if (readb(dev->pci_camera + CAMERA_FFFLAGS) & B_FIFO_OVERFLOW)
    dev->status |= CAMERA_FIFO_OVERFLOW;

  if (something_to_read) wake_up_interruptible(&dev->readq);

  /* something to transfer in hardawre FIFO? */
  fifo_count = readb(dev->pci_camera + CAMERA_FIFO_COUNT);

  PDEBUG(D_POLLING, "found %d further filled data buffers\n", fifo_count);

  if (!fifo_count) {
    restart_timer(dev);
    return;
  }

  /* sync descriptors for cpu */
  dma_sync_single_for_cpu(&dev->pci_dev->dev, dev->dma.descriptors_pci_address,
                          sizeof(struct dma_descriptor) * dev->dma.n_buffers,
                          DMA_TO_DEVICE);
  next_write = (dev->dma.ptrs.write_buf + 1) % dev->dma.n_buffers;

  PDEBUG(D_POLLING, "start syncing to device at buffer %d\n",
         dev->dma.ptrs.write_buf);
  buffers_to_transfer = 0;

  do {
    dma_sync_single_for_device(&dev->pci_dev->dev,
                               dev->dma.descriptors[dev->dma.ptrs.write_buf]
                               .pci_address,
                               dev->dma.buf_size, DMA_FROM_DEVICE);
    *((u32*)dev->dma.buffers[dev->dma.ptrs.write_buf])
      = ++dev->acquisition_count;

    dev->dma.descriptors[dev->dma.ptrs.write_buf].next_descriptor
      = (dev->dma.descriptors_pci_address
         + next_write * sizeof(struct dma_descriptor)) | 0x09;
    /* in pci, DMA from local to pci */
    buffers_to_transfer++;

    if (next_write == dev->dma.ptrs.read_buf) break;
    dev->dma.ptrs.write_buf = next_write;
    next_write = (next_write + 1) % dev->dma.n_buffers;
  } while (--fifo_count);

  PDEBUG(D_POLLING, "end syncing to device at buffer %d\n",
         dev->dma.ptrs.write_buf);

  if (next_write == dev->dma.ptrs.read_buf) dev->status |= CAMERA_BUFFER_FULL;

  /* let last buffer point to itself */
  dev->dma.descriptors[dev->dma.ptrs.write_buf].next_descriptor
    = (dev->dma.descriptors_pci_address
       + dev->dma.ptrs.write_buf * sizeof(struct dma_descriptor)) | 0x0B;
  /* in pci, DMA from local to pci, end of chain */

  /* sync descriptors for PLX9056 */
  dma_sync_single_for_device(&dev->pci_dev->dev,
                             dev->dma.descriptors_pci_address,
                           sizeof(struct dma_descriptor) * dev->dma.n_buffers,
                             DMA_TO_DEVICE);

  dev->status |= CAMERA_DMA_ACTIVE;
  plx9056_start_dma(dev, buffers_to_transfer);
  restart_timer(dev);
}

void restart_timer(struct dev_struct *dev) {
  dev->poll_timer.expires = jiffies + 2;
  add_timer(&dev->poll_timer);
  dev->timer_count++;
}
