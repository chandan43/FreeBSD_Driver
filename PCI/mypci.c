/*
 * Simple KLD to play with the PCI functions.
 *
 * Murray Stokely
 */

#include <sys/param.h>		/* defines used in kernel.h */
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/kernel.h>		/* types used in module initialization */
#include <sys/conf.h>		/* cdevsw struct */
#include <sys/uio.h>		/* uio struct */
#include <sys/malloc.h>
#include <sys/bus.h>		/* structs, prototypes for pci bus stuff and DEVMETHOD macros! */

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/pci/pcivar.h>	/* For pci_get macros! */
#include <dev/pci/pcireg.h>

/* The softc holds our per-instance data. */
struct mypci_softc {
	device_t	my_dev;
	struct cdev	*my_cdev;
};

/* Function prototypes */
static d_open_t		mypci_open;
static d_close_t	mypci_close;
static d_read_t		mypci_read;
static d_write_t	mypci_write;

/* Character device entry points */

static struct cdevsw mypci_cdevsw = {
	.d_version =	D_VERSION,
	.d_open =	mypci_open,
	.d_close =	mypci_close,
	.d_read =	mypci_read,
	.d_write =	mypci_write,
	.d_name =	"mypci",
};

/*
 * In the cdevsw routines, we find our softc by using the si_drv1 member
 * of struct cdev.  We set this variable to point to our softc in our
 * attach routine when we create the /dev entry.
 */

int
mypci_open(struct cdev *dev, int oflags, int devtype, struct thread *td)
{
	struct mypci_softc *sc;

	/* Look up our softc. */
	sc = dev->si_drv1;
	device_printf(sc->my_dev, "Opened successfully.\n");
	return (0);
}

int
mypci_close(struct cdev *dev, int fflag, int devtype, struct thread *td)
{
	struct mypci_softc *sc;

	/* Look up our softc. */
	sc = dev->si_drv1;
	device_printf(sc->my_dev, "Closed.\n");
	return (0);
}

int
mypci_read(struct cdev *dev, struct uio *uio, int ioflag)
{
	struct mypci_softc *sc;

	/* Look up our softc. */
	sc = dev->si_drv1;
	//device_printf(sc->my_dev, "Asked to read %d bytes.\n", uio->uio_resid);
	device_printf(sc->my_dev, "Asked to read %zd bytes.\n", uio->uio_resid);
	return (0);
}

int
mypci_write(struct cdev *dev, struct uio *uio, int ioflag)
{
	struct mypci_softc *sc;

	/* Look up our softc. */
	sc = dev->si_drv1;
	device_printf(sc->my_dev, "Asked to write %zd bytes.\n", uio->uio_resid);
	return (0);
}

/* PCI Support Functions */

/*
 * Compare the device ID of this device against the IDs that this driver
 * supports.  If there is a match, set the description and return success.
 */
static int
mypci_probe(device_t dev)
{

	device_printf(dev, "MyPCI Probe\nVendor ID : 0x%x\nDevice ID : 0x%x\n",
	    pci_get_vendor(dev), pci_get_device(dev));

	if (pci_get_vendor(dev) == 0x11c1) {
		printf("We've got the Winmodem, probe successful!\n");
		device_set_desc(dev, "WinModem");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

/* Attach function is only called if the probe is successful. */

static int
mypci_attach(device_t dev)
{
	struct mypci_softc *sc;

	printf("MyPCI Attach for : deviceID : 0x%x\n", pci_get_devid(dev));

	/* Look up our softc and initialize its fields. */
	sc = device_get_softc(dev);
	sc->my_dev = dev;

	/*
	 * Create a /dev entry for this device.  The kernel will assign us
	 * a major number automatically.  We use the unit number of this
	 * device as the minor number and name the character device
	 * "mypci<unit>".
	 */
	sc->my_cdev = make_dev(&mypci_cdevsw, device_get_unit(dev),
	    UID_ROOT, GID_WHEEL, 0600, "mypci%u", device_get_unit(dev));
	sc->my_cdev->si_drv1 = sc;
	printf("Mypci device loaded.\n");
	return (0);
}

/* Detach device. */

static int
mypci_detach(device_t dev)
{
	struct mypci_softc *sc;

	/* Teardown the state in our softc created in our attach routine. */
	sc = device_get_softc(dev);
	destroy_dev(sc->my_cdev);
	printf("Mypci detach!\n");
	return (0);
}

/* Called during system shutdown after sync. */

static int
mypci_shutdown(device_t dev)
{

	printf("Mypci shutdown!\n");
	return (0);
}

/*
 * Device suspend routine.
 */
static int
mypci_suspend(device_t dev)
{

	printf("Mypci suspend!\n");
	return (0);
}

/*
 * Device resume routine.
 */
static int
mypci_resume(device_t dev)
{

	printf("Mypci resume!\n");
	return (0);
}

static device_method_t mypci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		mypci_probe),
	DEVMETHOD(device_attach,	mypci_attach),
	DEVMETHOD(device_detach,	mypci_detach),
	DEVMETHOD(device_shutdown,	mypci_shutdown),
	DEVMETHOD(device_suspend,	mypci_suspend),
	DEVMETHOD(device_resume,	mypci_resume),

	DEVMETHOD_END
};

static devclass_t mypci_devclass;

DEFINE_CLASS_0(mypci, mypci_driver, mypci_methods, sizeof(struct mypci_softc));
DRIVER_MODULE(mypci, pci, mypci_driver, mypci_devclass, 0, 0);
