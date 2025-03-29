#include "include.h"

#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

const char *drivers_path = "/sys/bus/pci/drivers";
const char *devices_path = "/sys/bus/pci/devices";

const char *dpdk_drivers[] =
  { "uio_pci_generic", "igb_uio", "vfio-pci", NULL };
int dpdk_driver_num = 3;

int
is_dpdk_driver (char *module_name)
{
  int i;
  for (i = 0; i < dpdk_driver_num; i++)
    {
      if (! dpdk_drivers[i])
        continue;
      if (! strcmp (module_name, dpdk_drivers[i]))
        return 1;
    }
  return 0;
}

int
is_net_device (char *device_dirname)
{
  int ret;
  struct stat statbuf;
  char filename[256];
  snprintf (filename, sizeof (filename), "%s/net", device_dirname);
  ret = stat (filename, &statbuf);

  //DEBUG_SDPLANE_LOG (NETDEVICE, "filename: %s stat ret: %d",
  //                   filename, ret);

  if (ret == -1)
    return -1;
  if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
    return 1;
  return 0;
}

char *
get_net_devname (char *device_dirname)
{
  int ret;
  struct stat statbuf;
  char filename[256];
  DIR *dir;
  struct dirent *dirent;
  char *devname = NULL;
  snprintf (filename, sizeof (filename), "%s/net", device_dirname);
  //DEBUG_SDPLANE_LOG (NETDEVICE, "directory: %s", filename);

  dir = opendir (filename);
  if (dir == NULL)
    {
      //fprintf (stderr, "can't open directory: %s\n", filename);
      return NULL;
    }

  while ((dirent = readdir (dir)) != NULL)
    {
      /* everything starts with '.' is hidden. */
      if (dirent->d_name[0] == '.')
        continue;
      if (! devname)
        devname = strdup (dirent->d_name);
    }

  closedir (dir);

  return devname;
}

CLI_COMMAND2 (show_devices,
              "show devices",
              SHOW_HELP,
              "Devices information.\n")
{
  struct shell *shell = (struct shell *) context;
  DIR *dir;
  struct dirent *dirent;
  DIR *driver_dir;
  struct dirent *driver_ent;
  char devname[16];
  char devpath[256];
  char drvrpath[256];
  int i;

  dir = opendir (drivers_path);
  if (dir == NULL)
    {
      fprintf (shell->terminal, "can't open directory: %s%s",
               drivers_path, shell->NL);
      return;
    }

  while ((dirent = readdir (dir)) != NULL)
    {
      /* everything starts with '.' is hidden. */
      if (dirent->d_name[0] == '.')
        continue;

      char *module_name = &dirent->d_name[0];
      if (! is_dpdk_driver (module_name))
        continue;

      snprintf (drvrpath, sizeof (drvrpath), "%s/%s",
                drivers_path, module_name);

      driver_dir = opendir (drvrpath);
      if (driver_dir == NULL)
        {
          fprintf (shell->terminal, "can't open directory: %s%s",
                   drvrpath, shell->NL);
          return;
        }

      while ((driver_ent = readdir (driver_dir)) != NULL)
        {
          /* everything starts with '.' is hidden. */
          if (driver_ent->d_name[0] == '.')
            continue;

          char *device_name = &driver_ent->d_name[0];
          if (strlen (device_name) != 12 ||
              device_name[4] != ':' ||
              device_name[7] != ':' ||
              device_name[10] != '.')
            continue;

          fprintf (shell->terminal, "%s: %s%s%s", device_name,
                 "dpdk: ", module_name,
                 shell->NL);
        }

      closedir (driver_dir);
    }

  closedir (dir);

  dir = opendir (devices_path);
  if (dir == NULL)
    {
      fprintf (shell->terminal, "can't open directory: %s%s",
               devices_path, shell->NL);
      return;
    }

  char *is_netdev, *netdev_name;
  while ((dirent = readdir (dir)) != NULL)
    {
      /* everything starts with '.' is hidden. */
      if (dirent->d_name[0] == '.')
        continue;

      is_netdev = NULL;
      netdev_name = NULL;

      snprintf (devname, sizeof (devname), "%s", dirent->d_name);
      snprintf (devpath, sizeof (devpath), "%s/%s", devices_path, devname);
      if (is_net_device (devpath) == 1)
        {
          is_netdev = "net: ";
          netdev_name = get_net_devname (devpath);
        }

      if (is_netdev)
        fprintf (shell->terminal, "%s: %s%s%s", devname,
                 (is_netdev ? is_netdev : ""),
                 (netdev_name ? netdev_name : ""),
                 shell->NL);

      if (netdev_name)
        free (netdev_name);
    }

  closedir (dir);
}

int
pci_bus_number_spec (char *spec)
{
  return (! strcmp (spec, "<XXXX:XX:XX.X>"));
}

int
pci_bus_number_match (char *spec, char *word)
{
  if (strlen (word) == 7)
    {
      if (! isdigit (word[0])) return 0;
      if (! isdigit (word[1])) return 0;
      if (word[2] != ':') return 0;
      if (! isdigit (word[3])) return 0;
      if (! isdigit (word[4])) return 0;
      if (word[5] != '.') return 0;
      if (! isdigit (word[6])) return 0;
      return 1;
    }

  if (strlen (word) == 12)
    {
      if (! isdigit (word[0])) return 0;
      if (! isdigit (word[1])) return 0;
      if (! isdigit (word[2])) return 0;
      if (! isdigit (word[3])) return 0;
      if (word[4] != ':') return 0;
      if (! isdigit (word[5])) return 0;
      if (! isdigit (word[6])) return 0;
      if (word[7] != ':') return 0;
      if (! isdigit (word[8])) return 0;
      if (! isdigit (word[9])) return 0;
      if (word[10] != '.') return 0;
      if (! isdigit (word[11])) return 0;
      return 1;
    }

  return 0;
}

CLI_COMMAND2 (set_device_driver,
              "set device <WORD> "
              "driver (ixgbe|igb|igc|uio_pci_generic|igb_uio|vfio-pci|unbound)",
              SET_HELP,
              "Devices information.\n",
              "Specify PCI Bus Number for the device.\n",
              "DPDK driver information.\n",
              "module: ixgbe.\n",
              "module: igb.\n",
              "module: igc.\n",
              "module: uio_pci_generic.\n",
              "module: igb_uio.\n",
              "module: vfio-pci.\n",
              "unbind the device.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  char driver_bind_path[256];
  DIR *driver_dir;
  struct dirent *driver_ent;
  int fd;
  int ret;
  char pci_number[16];
  char driver_name[16];

  if (! pci_bus_number_match ("<XXXX:XX:XX.X>", argv[2]))
    {
      fprintf (shell->terminal, "unknown PCI number: %s%s",
               argv[2], shell->NL);
      return;
    }

  if (strlen (argv[2]) == 7)
    snprintf (pci_number, sizeof (pci_number), "0000:%s",  argv[2]);
  else
    snprintf (pci_number, sizeof (pci_number), "%s",  argv[2]);

  snprintf (driver_name , sizeof (driver_name), "%s", argv[4]);

  if (! strcmp (driver_name, "unbound"))
    snprintf (driver_bind_path, sizeof (driver_bind_path),
              "%s/%s/%s/unbind",
              devices_path, pci_number, "driver");
  else
    snprintf (driver_bind_path, sizeof (driver_bind_path),
              "%s/%s/driver_override",
              devices_path, pci_number);

  fd = open (driver_bind_path, O_WRONLY, 0);
  if (fd < 0)
    {
      fprintf (shell->terminal, "opening %s failed: %s%s",
               driver_bind_path, strerror (errno), shell->NL);
      return;
    }

  if (! strcmp (driver_name, "unbound"))
    ret = write (fd, pci_number, strlen (pci_number));
  else
    ret = write (fd, driver_name, strlen (driver_name));
  if (ret < 0)
    {
      fprintf (shell->terminal, "write %s to %s failed: %s%s",
               pci_number, driver_bind_path, strerror (errno), shell->NL);
      return;
    }

  fprintf (shell->terminal, "bind %s to dev: %s%s",
           argv[4], argv[2], shell->NL);
}

void
dpdk_devbind_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_devices);
  INSTALL_COMMAND2 (cmdset, set_device_driver);
}


