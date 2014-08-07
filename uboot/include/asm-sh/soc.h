/*
 * (C) Copyright 2008-2010 STMicroelectronics.
 *
 * Sean McGoogan <Sean.McGoogan@st.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#ifndef _SOC_H_
#define	_SOC_H_


/*
 * PIO data types
 */
struct stx7108_pioalt_pad_cfg
{
	int oe:2;
	int pu:2;
	int od:2;
};
extern const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_in;
extern const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_out;
extern const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_od;
extern const struct stx7108_pioalt_pad_cfg stx7108_pioalt_pad_bidir;


/*
 * PIO initialization functions
 */
extern void stx7108_pioalt_pad(
	int port,
	const int pin,
	const struct stx7108_pioalt_pad_cfg * const cfg);
extern void stx7108_pioalt_select(
	const int port,
	const int pin,
	const int alt);


/*
 * STMAC data types
 */
enum fli7510_ethernet_mode
{
	fli7510_ethernet_mii,
	fli7510_ethernet_gmii,
	fli7510_ethernet_rmii,
	fli7510_ethernet_reverse_mii
};
enum fli7540_ethernet_mode
{
	fli7540_ethernet_mii,
	fli7540_ethernet_gmii,
	fli7540_ethernet_rmii,
	fli7540_ethernet_reverse_mii
};
enum stx5206_ethernet_mode
{
	stx5206_ethernet_mii,
	stx5206_ethernet_rmii,
	stx5206_ethernet_reverse_mii
};
struct stx7108_ethernet_config
{
	enum {
		stx7108_ethernet_mode_mii,
		stx7108_ethernet_mode_gmii,
		stx7108_ethernet_mode_gmii_gtx,
		stx7108_ethernet_mode_rmii,
		stx7108_ethernet_mode_reverse_mii
	} mode;
	int ext_clk;
	int phy_bus;
};


/*
 * common call-back functions for STMAC.
 */
extern int  stmac_default_pbl (void);
extern void stmac_set_mac_speed (int speed);
extern void stmac_phy_reset (void);


/*
 * STMAC initialization functions.
 */
extern void stx5206_configure_ethernet(
	const enum stx5206_ethernet_mode mode,
	const int ext_clk,
	const int phy_bus);
extern void stx7108_configure_ethernet(
	const int port,
	const struct stx7108_ethernet_config * const config);
extern void stx7141_configure_ethernet(
	const int port,
	const int reverse_mii,
	const int mode,
	const int phy_bus);
extern void stx7200_configure_ethernet (
	int mac, int rmii, int ext_clk, int phy_bus);
extern void fli7510_configure_ethernet(
	const enum fli7510_ethernet_mode mode,
	const int ext_clk,
	const int phy_bus);
extern void fli7540_configure_ethernet(
	const enum fli7540_ethernet_mode mode,
	const int ext_clk,
	const int phy_bus);


/*
 * SATA initialization functions.
 */
extern void stm_sata_miphy_init(void);
extern void stm_sata_miphy_deassert_des_reset(void);
extern int  stm_sata_probe(void);

extern void stb7100_sata_init(void);
extern void stx7105_configure_sata(void);
extern void stx7141_configure_sata(void);
extern void stx7200_configure_sata(void);


/*
 * USB data types
 */
enum fli7510_usb_ovrcur_mode {
	fli7510_usb_ovrcur_disabled,
	fli7510_usb_ovrcur_active_high,
	fli7510_usb_ovrcur_active_low,
};
enum fli7540_usb_ovrcur_mode {
	fli7540_usb_ovrcur_disabled,
	fli7540_usb_ovrcur_active_high,
	fli7540_usb_ovrcur_active_low,
};


/*
 * USB initialization functions.
 */
extern void stb7100_usb_init(void);
extern void stx5197_usb_init(void);
extern void stx5206_usb_init(void);
extern int  stx7105_usb_init(int port, int over_current, int power_ctrl);
extern int  stx7108_usb_init(const int port);
extern void stx7111_usb_init(void);
extern void stx7141_usb_init(void);
extern void stx7200_usb_init(void);
extern void fli7510_usb_init(const enum fli7510_usb_ovrcur_mode ovrcur_mode);
extern void fli7540_usb_init(const int port, const enum fli7540_usb_ovrcur_mode ovrcur_mode);


/*
 * SPI initialization functions.
 */
extern void		stx7108_configure_spi(void);


/*
 * Software "bit-banging" functions for SPI accesses.
 */
extern void		stx7105_spi_scl(const int val);
extern void		stx7105_spi_sda(const int val);
extern unsigned char	stx7105_spi_read(void);

extern void		stx7108_spi_scl(const int val);
extern void		stx7108_spi_sda(const int val);
extern unsigned char	stx7108_spi_read(void);

extern void		stx7111_spi_scl(const int val);
extern void		stx7111_spi_sda(const int val);
extern unsigned char	stx7111_spi_read(void);

extern void		fli7510_spi_scl(const int val);
extern void		fli7510_spi_sda(const int val);
extern unsigned char	fli7510_spi_read(void);

extern void		fli7540_spi_scl(const int val);
extern void		fli7540_spi_sda(const int val);
extern unsigned char	fli7540_spi_read(void);


/*
 * I2C initialization functions.
 */
extern void		stx7108_configure_i2c(void);


/*
 * Software "bit-banging" functions for I2C accesses.
 */
extern void		stx7105_i2c_scl(const int val);
extern void		stx7105_i2c_sda(const int val);
extern int		stx7105_i2c_read(void);

extern void		stx7108_i2c_scl(const int val);
extern void		stx7108_i2c_sda(const int val);
extern int		stx7108_i2c_read(void);

extern void		fli7510_i2c_scl(const int val);
extern void		fli7510_i2c_sda(const int val);
extern int		fli7510_i2c_read(void);

extern void		fli7540_i2c_scl(const int val);
extern void		fli7540_i2c_sda(const int val);
extern int		fli7540_i2c_read(void);


#endif	/* _SOC_H_ */

