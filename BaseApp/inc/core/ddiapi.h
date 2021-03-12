

#ifndef DDI_API_H_
#define DDI_API_H_

extern struct  __core *core ;



#define MAX_BUFFER_LENGTH                   512


#define ddi_misc_appprinf       (core->CoreApi_ddi_misc_appprinf)
#define ddi_misc_msleep         (core->CoreApi_ddi_misc_msleep)
#define ddi_misc_set_time       (core->CoreApi_ddi_misc_set_time)
#define ddi_misc_get_time       (core->CoreApi_ddi_misc_get_time)
#define ddi_misc_get_tick       (core->CoreApi_ddi_misc_get_tick)  //MTK 1 ticks = 4.615ms
#define ddi_misc_read_dsn     (core->CoreApi_ddi_misc_read_dsn)
#define ddi_misc_sleep          (core->CoreApi_ddi_misc_sleep)
#define ddi_misc_get_firmwareversion          (core->CoreApi_ddi_misc_get_firmwareversion)
#define ddi_misc_bat_status     (core->CoreApi_ddi_misc_bat_status)
#define ddi_misc_poweroff       (core->CoreApi_ddi_misc_poweroff)
#define k_mallocapp             (core->CoreApi_k_mallocapp)
#define k_freeapp               (core->CoreApi_k_freeapp)
#define ddi_misc_sleep_with_alarm               (core->CoreApi_ddi_misc_sleep_with_alarm)
#define ddi_misc_reboot          (core->CoreApi_ddi_misc_reboot)
#define ddi_misc_probe_dev      (core->CoreApi_ddi_misc_probe_dev)
#define ddi_user_timer_open        (core->CoreApi_ddi_user_timer_open)
#define ddi_user_timer_close       (core->CoreApi_ddi_user_timer_close)
#define ddi_pci_readtamperstatus       (core->CoreApi_ddi_pci_readtamperstatus)
#define ddi_pci_getcurrenttamperstatus (core->CoreApi_ddi_pci_getcurrenttamperstatus)
#define ddi_pci_unlock                        (core->CoreApi_ddi_pci_unlock)
#define ddi_pci_opendryice             (core->CoreApi_ddi_pci_opendryice)
#define ddi_pci_getdryiceconfig        (core->CoreApi_ddi_pci_getdryiceconfig)
#define ddi_pci_getspstatuspointer (core->CoreApi_ddi_pci_getspstatuspointer)
#define ddi_misc_ioctl             (core->CoreApi_ddi_misc_ioctl)


#define ddi_com_open              (core->CoreApi_ddi_com_open)
#define ddi_com_close             (core->CoreApi_ddi_com_close)
#define ddi_com_clear             (core->CoreApi_ddi_com_clear)
#define ddi_com_read              (core->CoreApi_ddi_com_read)
#define ddi_com_write             (core->CoreApi_ddi_com_write)
#define ddi_com_ioctl             (core->CoreApi_ddi_com_ioctl)
#define ddi_com_tcdrain        (core->CoreApi_ddi_com_tcdrain)

#define ddi_mag_open              (core->CoreApi_ddi_mag_open)
#define ddi_mag_close             (core->CoreApi_ddi_mag_close)
#define ddi_mag_clear             (core->CoreApi_ddi_mag_clear)
#define ddi_mag_read              (core->CoreApi_ddi_mag_read)
#define ddi_mag_ioctl             (core->CoreApi_ddi_mag_ioctl)


#define ddi_iccpsam_open           (core->CoreApi_ddi_iccpsam_open)
#define ddi_iccpsam_close          (core->CoreApi_ddi_iccpsam_close)
#define ddi_iccpsam_poweron        (core->CoreApi_ddi_iccpsam_poweron)
#define ddi_iccpsam_poweroff       (core->CoreApi_ddi_iccpsam_poweroff)
#define ddi_iccpsam_get_status     (core->CoreApi_ddi_iccpsam_get_status)
#define ddi_iccpsam_exchange_apdu  (core->CoreApi_ddi_iccpsam_exchange_apdu)
#define ddi_iccpsam_ioctl          (core->CoreApi_ddi_iccpsam_ioctl)


#define ddi_rf_open                 (core->CoreApi_ddi_rf_open)
#define ddi_rf_close                (core->CoreApi_ddi_rf_close)
#define ddi_rf_poweron              (core->CoreApi_ddi_rf_poweron)
#define ddi_rf_poweroff             (core->CoreApi_ddi_rf_poweroff)
#define ddi_rf_get_status           (core->CoreApi_ddi_rf_get_status)
#define ddi_rf_activate             (core->CoreApi_ddi_rf_activate)
#define ddi_rf_exchange_apdu        (core->CoreApi_ddi_rf_exchange_apdu)
#define ddi_rf_remove               (core->CoreApi_ddi_rf_remove)
#define ddi_rf_ioctl                (core->CoreApi_ddi_rf_ioctl)

#define ddi_thmprn_open                (core->CoreApi_ddi_thmprn_open)
#define ddi_thmprn_close               (core->CoreApi_ddi_thmprn_close)
#define ddi_thmprn_feed_paper          (core->CoreApi_ddi_thmprn_feed_paper)
#define ddi_thmprn_print_image         (core->CoreApi_ddi_thmprn_print_image)
#define ddi_thmprn_print_image_file    (core->CoreApi_ddi_thmprn_print_image_file)
#define ddi_thmprn_print_text          (core->CoreApi_ddi_thmprn_print_text)
#define ddi_thmprn_print_comb_text     (core->CoreApi_ddi_thmprn_print_comb_text)
#define ddi_thmprn_get_status          (core->CoreApi_ddi_thmprn_get_status)
#define ddi_thmprn_ioctl               (core->CoreApi_ddi_thmprn_ioctl)
#define ddi_thmprn_esc_p               (core->CoreApi_ddi_thmprn_esc_p)
#define ddi_thmprn_esc_loop            (core->CoreApi_ddi_thmprn_esc_loop)

#define ddi_thmprn_esc_init            (core->CoreApi_ddi_thmprn_esc_init)

#define ddi_key_open                   (core->CoreApi_ddi_key_open)
#define ddi_key_close                  (core->CoreApi_ddi_key_close)
#define ddi_key_clear                  (core->CoreApi_ddi_key_clear)
#define ddi_key_read                   (core->CoreApi_ddi_key_read)
#define ddi_key_ioctl                  (core->CoreApi_ddi_key_ioctl)
#define ddi_key_read_withoutdelay      (core->CoreApi_ddi_key_read_withoutdelay)


#define ddi_lcd_open                   (core->CoreApi_ddi_lcd_open)
#define ddi_lcd_close                  (core->CoreApi_ddi_lcd_close)
#define ddi_lcd_fill_rect              (core->CoreApi_ddi_lcd_fill_rect)
#define ddi_lcd_clear_rect             (core->CoreApi_ddi_lcd_clear_rect)
#define ddi_lcd_show_text              (core->CoreApi_ddi_lcd_show_text)
#define ddi_lcd_show_picture           (core->CoreApi_ddi_lcd_show_picture)
#define ddi_lcd_monochrome		    (core->CoreApi_ddi_lcd_monochrome)
#define ddi_lcd_show_picture_file      (core->CoreApi_ddi_lcd_show_picture_file)
#define ddi_lcd_show_pixel             (core->CoreApi_ddi_lcd_show_pixel)
#define ddi_lcd_show_line              (core->CoreApi_ddi_lcd_show_line)
#define ddi_lcd_show_rect              (core->CoreApi_ddi_lcd_show_rect)
#define ddi_lcd_extract_rect           (core->CoreApi_ddi_lcd_extract_rect)
#define ddi_lcd_ioctl                  (core->CoreApi_ddi_lcd_ioctl)
#define ddi_lcd_brush_screen           (core->CoreApi_ddi_lcd_brush_screen)

#define ddi_audio_open                 (core->CoreApi_ddi_audio_open)
#define ddi_audio_close                (core->CoreApi_ddi_audio_close)
#define ddi_audio_play                 (core->CoreApi_ddi_audio_play)
#define ddi_audio_playcharacters  (core->CoreApi_ddi_audio_playcharacters)
#define ddi_audio_stop                 (core->CoreApi_ddi_audio_stop)
#define ddi_audio_pause                (core->CoreApi_ddi_audio_pause)
#define ddi_audio_resumeplay             (core->CoreApi_ddi_audio_resumeplay)
#define ddi_audio_ioctl                (core->CoreApi_ddi_audio_ioctl)

#define ddi_innerkey_open              (core->CoreApi_ddi_innerkey_open)//
#define ddi_innerkey_close             (core->CoreApi_ddi_innerkey_close)
#define ddi_innerkey_inject            (core->CoreApi_ddi_innerkey_inject)
#define ddi_innerkey_delete             (core->CoreApi_ddi_innerkey_delete)
#define ddi_innerkey_encrypt           (core->CoreApi_ddi_innerkey_encrypt)
#define ddi_innerkey_decrypt           (core->CoreApi_ddi_innerkey_decrypt)
#define ddi_innerkey_getHW21SNinfo   (core->CoreApi_ddi_innerkey_getHW21SNinfo)
#define ddi_innerkey_ioctl             (core->CoreApi_ddi_innerkey_ioctl)

#define ddi_file_create                (core->CoreApi_ddi_file_create)
#define ddi_file_write                 (core->CoreApi_ddi_file_write)
#define ddi_file_read                  (core->CoreApi_ddi_file_read)
#define ddi_file_insert                (core->CoreApi_ddi_file_insert)
#define ddi_file_delete                (core->CoreApi_ddi_file_delete)
#define ddi_file_rename                (core->CoreApi_ddi_file_rename)
#define ddi_file_getlength             (core->CoreApi_ddi_file_getlength)
#define ddi_file_veritysignature       (core->CoreApi_ddi_file_veritysignature)
#define ddi_static_vfs_getarea         (core->CoreApi_ddi_static_vfs_getarea)
#define ddi_vfs_open                   (core->CoreApi_ddi_vfs_open)
#define ddi_vfs_close                  (core->CoreApi_ddi_vfs_close)
#define ddi_vfs_seek                   (core->CoreApi_ddi_vfs_seek)
#define ddi_vfs_tell                   (core->CoreApi_ddi_vfs_tell)
#define ddi_vfs_read                   (core->CoreApi_ddi_vfs_read)
#define ddi_vfs_write                  (core->CoreApi_ddi_vfs_write)
#define ddi_vfs_sync                   (core->CoreApi_ddi_vfs_sync)
#define ddi_vfs_deletefile             (core->CoreApi_ddi_vfs_deletefile)
#define ddi_vfs_renamefile             (core->CoreApi_ddi_vfs_renamefile)
#define ddi_vfs_free_space             (core->CoreApi_ddi_vfs_free_space)
#define ddi_vfs_factory_initial        (core->CoreApi_ddi_vfs_factory_initial)
#define ddi_flash_read         (core->CoreApi_ddi_flash_read)
#define ddi_flash_write        (core->CoreApi_ddi_flash_write)

#define ddi_bt_open                    (core->CoreApi_ddi_bt_open)
#define ddi_bt_close                   (core->CoreApi_ddi_bt_close)
#define ddi_bt_disconnect              (core->CoreApi_ddi_bt_disconnect)
#define ddi_bt_write                   (core->CoreApi_ddi_bt_write)
#define ddi_bt_read                    (core->CoreApi_ddi_bt_read)
#define ddi_bt_get_status              (core->CoreApi_ddi_bt_get_status)
#define ddi_bt_ioctl                   (core->CoreApi_ddi_bt_ioctl)

#define ddi_gprs_open 				   (core->CoreApi_ddi_gprs_open)
#define ddi_gprs_close 				   (core->CoreApi_ddi_gprs_close)
#define ddi_gprs_get_signalquality     (core->CoreApi_ddi_gprs_get_signalquality)
#define ddi_gprs_telephony_dial        (core->CoreApi_ddi_gprs_telephony_dial)
#define ddi_gprs_telephony_hangup      (core->CoreApi_ddi_gprs_telephony_hangup)
#define ddi_gprs_telephony_answer      (core->CoreApi_ddi_gprs_telephony_answer)
#define ddi_gprs_get_pdpstatus 		   (core->CoreApi_ddi_gprs_get_pdpstatus)
#define ddi_gprs_set_apn 			   (core->CoreApi_ddi_gprs_set_apn)
#define ddi_gprs_socket_create		   (core->CoreApi_ddi_gprs_socket_create)
#define ddi_gprs_socket_send		   (core->CoreApi_ddi_gprs_socket_send)
#define ddi_gprs_socket_recv		   (core->CoreApi_ddi_gprs_socket_recv)
#define ddi_gprs_socket_close		   (core->CoreApi_ddi_gprs_socket_close)
#define ddi_gprs_socket_get_status 	   (core->CoreApi_ddi_gprs_socket_get_status)
#define ddi_gprs_get_siminfo		   (core->CoreApi_ddi_gprs_get_siminfo)
#define ddi_gprs_get_imei 		(core->CoreApi_ddi_gprs_get_imei)
#define ddi_gprs_get_basestation_info   (core->CoreApi_ddi_gprs_get_basestation_info)
#define ddi_gprs_ioctl			 (core->CoreApi_ddi_gprs_ioctl)

#define ddi_wifi_open					(core->CoreApi_ddi_wifi_open)
#define ddi_wifi_close					(core->CoreApi_ddi_wifi_close)
#define ddi_wifi_scanap_start					(core->CoreApi_ddi_wifi_scanap_start)
#define ddi_wifi_scanap_status              (core->CoreApi_ddi_wifi_scanap_status)
#define ddi_wifi_connectap_start			(core->CoreApi_ddi_wifi_connectap_start)
#define ddi_wifi_connectap_status 	(core->CoreApi_ddi_wifi_connectap_status)
#define ddi_wifi_disconnectap			(core->CoreApi_ddi_wifi_disconnectap)
#define ddi_wifi_socket_create_start			(core->CoreApi_ddi_wifi_socket_create_start)
#define ddi_wifi_socket_get_status   (core->CoreApi_ddi_wifi_socket_get_status)
#define ddi_wifi_socket_send			(core->CoreApi_ddi_wifi_socket_send)
#define ddi_wifi_socket_recv			(core->CoreApi_ddi_wifi_socket_recv)
#define ddi_wifi_socket_close			(core->CoreApi_ddi_wifi_socket_close)
#define ddi_wifi_get_signal				(core->CoreApi_ddi_wifi_get_signal)
#define ddi_wifi_ioctl					(core->CoreApi_ddi_wifi_ioctl)
#define ddi_wifi_start_airkiss			(core->CoreApi_ddi_wifi_start_airkiss)
#define ddi_wifi_get_airkiss_status		(core->CoreApi_ddi_wifi_get_airkiss_status)
#define ddi_wifi_get_airkiss_config		(core->CoreApi_ddi_wifi_get_airkiss_config)
#define ddi_wifi_stop_airkiss			(core->CoreApi_ddi_wifi_stop_airkiss)


#define ddi_led_open                   (core->CoreApi_ddi_led_open)
#define ddi_led_close                  (core->CoreApi_ddi_led_close)
#define ddi_led_sta_set                (core->CoreApi_ddi_led_sta_set)
#define ddi_led_ioctl                  (core->CoreApi_ddi_led_ioctl)

#define ddi_dukpt_open                (core->CoreApi_ddi_dukpt_open)
#define ddi_dukpt_close               (core->CoreApi_ddi_dukpt_close)
#define ddi_dukpt_inject              (core->CoreApi_ddi_dukpt_inject)
#define ddi_dukpt_encrypt             (core->CoreApi_ddi_dukpt_encrypt)
#define ddi_dukpt_decrypt             (core->CoreApi_ddi_dukpt_decrypt)
#define ddi_dukpt_getksn              (core->CoreApi_ddi_dukpt_getksn)
#define ddi_dukpt_ioctl               (core->CoreApi_ddi_dukpt_ioctl)

//ssl socket
#define ddi_mbedtls_ssl_open		  (core->CoreApi_ddi_mbedtls_ssl_open)
#define ddi_mbedtls_ssl_close		  (core->CoreApi_ddi_mbedtls_ssl_close)
#define ddi_mbedtls_ssl_set_ca_cert	  (core->CoreApi_ddi_mbedtls_ssl_set_ca_cert)
#define ddi_mbedtls_ssl_set_client_private_key (core->CoreApi_ddi_mbedtls_ssl_set_client_private_key)
#define ddi_mbedtls_ssl_set_client_cert (core->CoreApi_ddi_mbedtls_ssl_set_client_cert)
#define ddi_mbedtls_ssl_handshake     (core->CoreApi_ddi_mbedtls_ssl_handshake)
#define ddi_mbedtls_ssl_send		  (core->CoreApi_ddi_mbedtls_ssl_send)
#define ddi_mbedtls_ssl_recv		  (core->CoreApi_ddi_mbedtls_ssl_recv)
#define ddi_mbedtls_ssl_set_host_name (core->CoreApi_ddi_mbedtls_ssl_set_host_name)
#define ddi_mbedtls_verify_certchain   (core->CoreApi_ddi_mbedtls_verify_certchain)
#define ddi_mbedtls_parse_cert_info   (core->CoreApi_ddi_mbedtls_parse_cert_info)
#define ddi_mbedtls_pk_verify_sign    (core->CoreApi_ddi_mbedtls_pk_verify_sign)
#define ddi_mbedtls_sk_verify_sign     (core->CoreApi_ddi_mbedtls_sk_verify_sign)
#define ddi_mbedtls_pk_sign  		  (core->CoreApi_ddi_mbedtls_pk_sign)
#define ddi_mbedtls_sk_sign		 (core->CoreApi_ddi_mbedtls_sk_sign)
#define ddi_mbedtls_ioctl 	(core->CoreApi_ddi_mbedtls_ioctl)

//electroinc signature
#define ddi_elec_sign_open			  (core->CoreApi_ddi_elec_sign_open)
#define ddi_elec_sign_get_data_size	  (core->CoreApi_ddi_elec_sign_get_data_size)
#define ddi_elec_sign_get_data	      (core->CoreApi_ddi_elec_sign_get_data)
#define ddi_elec_sign_process          (core->CoreApi_ddi_elec_sign_process)
#define ddi_elec_sign_start	          (core->CoreApi_ddi_elec_sign_start)
#define ddi_elec_sign_stop            (core->CoreApi_ddi_elec_sign_stop)
#define ddi_elec_sign_close	          (core->CoreApi_ddi_elec_sign_close)
#define ddi_elec_sign_draw_text		  (core->CoreApi_ddi_elec_sign_draw_text)
#define ddi_elec_sign_ioctl 		  (core->CoreApi_ddi_elec_sign_ioctl)

#define ddi_arith_hash                (core->CoreApi_ddi_arith_hash)
#define ddi_arith_des                 (core->CoreApi_ddi_arith_des)
#define ddi_arith_getrand             (core->CoreApi_ddi_arith_getrand)
#define ddi_arith_rsaencrypt          (core->CoreApi_ddi_arith_rsaencrypt)
#define ddi_arith_rsadecrypt          (core->CoreApi_ddi_arith_rsadecrypt)
#define ddi_arith_rsaskencrypt        (core->CoreApi_ddi_arith_rsaskencrypt)
#define ddi_arith_rsaskdecrypt        (core->CoreApi_ddi_arith_rsaskdecrypt)
#define ddi_arith_rsarecover          (core->CoreApi_ddi_arith_rsarecover)
#define ddi_arith_sm2veritysignature  (core->CoreApi_ddi_arith_sm2veritysignature)
#define ddi_arith_sm3                 (core->CoreApi_ddi_arith_sm3)
#define ddi_arith_sm4                 (core->CoreApi_ddi_arith_sm4)
#define ddi_arith_file_hash           (core->CoreApi_ddi_arith_file_hash)
#define ddi_arith_md5                   (core->CoreApi_ddi_arith_md5)

#define ddi_watchdog_open			  (core->CoreApi_ddi_watchdog_open)
#define ddi_watchdog_close			  (core->CoreApi_ddi_watchdog_close)
#define ddi_watchdog_feed			  (core->CoreApi_ddi_watchdog_feed)

//生成二维码
#define ddi_QRcode_encodeString       (core->CoreApi_ddi_QRcode_encodeString)
#define ddi_QRcode_free         				(core->CoreApi_ddi_QRcode_free)
#define ddi_scanner_open				(core->CoreApi_ddi_scanner_open)
#define ddi_scanner_start				(core->CoreApi_ddi_scanner_start)
#define ddi_scanner_abort				(core->CoreApi_ddi_scanner_abort)
#define ddi_scanner_getdata				(core->CoreApi_ddi_scanner_getdata)
#define ddi_scanner_close				(core->CoreApi_ddi_scanner_close)
#define ddi_scanner_ioctl                           (core->CoreApi_ddi_scanner_ioctl)
#define ddi_Barcode_encodeString       (core->CoreApi_ddi_Barcode_encodeString)
#define ddi_Barcode_free                      (core->CoreApi_ddi_Barcode_free)

// sp manage
#define ddi_spnormalmanage_reset  (core->CoreApi_ddi_spnormalmanage_reset)
#define ddi_spnormalmanage_hardwarereset (core->CoreApi_ddi_spnormalmanage_hardwarereset)
#define ddi_spnormalmanage_setsleeptime (core->CoreApi_ddi_spnormalmanage_setsleeptime)
#define ddi_spnormalmanage_getsleeptime (core->CoreApi_ddi_spnormalmanage_getsleeptime)
#define ddi_spnormalmanage_intosleep (core->CoreApi_ddi_spnormalmanage_intosleep)
#define ddi_spsecuremanage_getspstatus (core->CoreApi_ddi_spsecuremanage_getspstatus)
#define ddi_spfirmwaremanage_intobootmode (core->CoreApi_ddi_spfirmwaremanage_intobootmode)
#define ddi_spfirmwaremanage_exitbootmode (core->CoreApi_ddi_spfirmwaremanage_exitbootmode)
#define ddi_spfirmwaremanage_update (core->CoreApi_ddi_spfirmwaremanage_update)
#define ddi_spfirmwaremanage_intocdcdatatospimode (core->CoreApi_ddi_spfirmwaremanage_intocdcdatatospimode)
#define ddi_spfirmwaremanage_spireadcdcdata (core->CoreApi_ddi_spfirmwaremanage_spireadcdcdata)
#define ddi_spfirmwaremanage_getspcertinfo (core->CoreApi_ddi_spfirmwaremanage_getspcertinfo)
#define ddi_spnormalmanage_setdevicesn (core->CoreApi_ddi_spnormalmanage_setdevicesn)
#define ddi_spnormalmanage_getdevicesn (core->CoreApi_ddi_spnormalmanage_getdevicesn)
#define ddi_spnormalmanage_getversion (core->CoreApi_ddi_spnormalmanage_getversion)
#define ddi_spnormalmanage_randnum (core->CoreApi_ddi_spnormalmanage_randnum)
#define ddi_spsecuremanage_gettamperinfo (core->CoreApi_ddi_spsecuremanage_gettamperinfo)
#define ddi_spsecuremanage_requestsecuredata (core->CoreApi_ddi_spsecuremanage_requestsecuredata)
#define ddi_spsecuremanage_setsecuredata (core->CoreApi_ddi_spsecuremanage_setsecuredata)
#define ddi_spnormalmanage_beep (core->CoreApi_ddi_spnormalmanage_beep)
#define ddi_spsecuremanage_getdebugflag (core->CoreApi_ddi_spsecuremanage_getdebugflag)
#define ddi_spsecuremanage_opendryice (core->CoreApi_ddi_spsecuremanage_opendryice)
#define ddi_dev_apspcomm_command (core->CoreApi_ddi_dev_apspcomm_command)
#define ddi_secureprevent_getspinfo (core->CoreApi_ddi_secureprevent_getspinfo)
#define ddi_secureprevent_sendbinddatatosp (core->CoreApi_ddi_secureprevent_sendbinddatatosp)
#define ddi_secureprevent_saveappcertinfo (core->CoreApi_ddi_secureprevent_saveappcertinfo)
#define ddi_secureprevent_readappcertinfo (core->CoreApi_ddi_secureprevent_readappcertinfo)
#define ddi_secureprevent_getsnkeycheckvalue  (core->CoreApi_ddi_secureprevent_getsnkeycheckvalue)
#define ddi_spsecuremanage_dump_memroyinfo (core->CoreApi_ddi_spsecuremanage_dump_memoryinfo)



//其他杂项

typedef struct _PARAM_BTFIRMWAREUPDATE
{ 
    u8 filename[20];          //0x60 for KEYA，0x61 for KEYB
    u32 offset;
}PARAM_BTFIRMWAREUPDATE;





typedef struct _PCI_CORE_VERINFO
{
	u8 ver[20];
}PCI_CORE_VERINFO;


extern s32 bootdownload_fread_specifyaddr(u8 *data,s32 fileoffset,u32 length,u32 fileorgaddr);
extern void ddi_core_printf(s8 *fmt,...);
extern void ddi_misc_get_appver(u8 *p);
extern s32 ddi_misc_set_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset);
extern s32 ddi_misc_get_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset);


#endif





