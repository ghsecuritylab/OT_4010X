

#ifndef WPS_H
#define WPS_H

#include "wps_defs.h"

enum wsc_op_code {
	WSC_UPnP = 0 /* No OP Code in UPnP transport */,
	WSC_Start = 0x01,
	WSC_ACK = 0x02,
	WSC_NACK = 0x03,
	WSC_MSG = 0x04,
	WSC_Done = 0x05,
	WSC_FRAG_ACK = 0x06
};

struct wps_registrar;
struct upnp_wps_device_sm;
struct wps_er;

struct wps_credential {
	u8 ssid[32];
	size_t ssid_len;
	u16 auth_type;
	u16 encr_type;
	u8 key_idx;
	u8 key[64];
	size_t key_len;
	u8 mac_addr[ETH_ALEN];
	const u8 *cred_attr;
	size_t cred_attr_len;
};

#define WPS_DEV_TYPE_LEN 8
#define WPS_DEV_TYPE_BUFSIZE 21

struct wps_device_data {
	u8 mac_addr[ETH_ALEN];
	char *device_name;
	char *manufacturer;
	char *model_name;
	char *model_number;
	char *serial_number;
        u8 pri_dev_type[WPS_DEV_TYPE_LEN];
	u16 categ;
	u32 oui;
	u16 sub_categ;
	u32 os_version;
	u8 rf_bands;
};

struct wps_config {
	/**
	 * wps - Pointer to long term WPS context
	 */
	struct wps_context *wps;

	/**
	 * registrar - Whether this end is a Registrar
	 */
	int registrar;

	/**
	 * pin - Enrollee Device Password (%NULL for Registrar or PBC)
	 */
	const u8 *pin;

	/**
	 * pin_len - Length on pin in octets
	 */
	size_t pin_len;

	/**
	 * pbc - Whether this is protocol run uses PBC
	 */
	int pbc;

	/**
	 * assoc_wps_ie: (Re)AssocReq WPS IE (in AP; %NULL if not AP)
	 */
	const struct wpabuf *assoc_wps_ie;

	/**
	 * new_ap_settings - New AP settings (%NULL if not used)
	 *
	 * This parameter provides new AP settings when using a wireless
	 * stations as a Registrar to configure the AP. %NULL means that AP
	 * will not be reconfigured, i.e., the station will only learn the
	 * current AP settings by using AP PIN.
	 */
	const struct wps_credential *new_ap_settings;

	/**
	 * peer_addr: MAC address of the peer in AP; %NULL if not AP
	 */
	const u8 *peer_addr;

	/**
	 * use_psk_key - Use PSK format key in Credential
	 *
	 * Force PSK format to be used instead of ASCII passphrase when
	 * building Credential for an Enrollee. The PSK value is set in
	 * struct wpa_context::psk.
	 */
	int use_psk_key;

	/**
	 * dev_pw_id - Device Password ID for Enrollee when PIN is used
	 */
	u16 dev_pw_id;
};

struct wps_data * wps_init(const struct wps_config *cfg);

void wps_deinit(struct wps_data *data);

enum wps_process_res {
	/**
	 * WPS_DONE - Processing done
	 */
	WPS_DONE,

	/**
	 * WPS_CONTINUE - Processing continues
	 */
	WPS_CONTINUE,

	/**
	 * WPS_FAILURE - Processing failed
	 */
	WPS_FAILURE,

	/**
	 * WPS_PENDING - Processing continues, but waiting for an external
	 *	event (e.g., UPnP message from an external Registrar)
	 */
	WPS_PENDING
};
enum wps_process_res wps_process_msg(struct wps_data *wps,
				     enum wsc_op_code op_code,
				     const struct wpabuf *msg);

struct wpabuf * wps_get_msg(struct wps_data *wps, enum wsc_op_code *op_code);

int wps_is_selected_pbc_registrar(const struct wpabuf *msg);
int wps_is_selected_pin_registrar(const struct wpabuf *msg);
const u8 * wps_get_uuid_e(const struct wpabuf *msg);

struct wpabuf * wps_build_assoc_req_ie(enum wps_request_type req_type);
struct wpabuf * wps_build_assoc_resp_ie(void);
struct wpabuf * wps_build_probe_req_ie(int pbc, struct wps_device_data *dev,
				       const u8 *uuid,
				       enum wps_request_type req_type);


struct wps_registrar_config {
	/**
	 * new_psk_cb - Callback for new PSK
	 * @ctx: Higher layer context data (cb_ctx)
	 * @mac_addr: MAC address of the Enrollee
	 * @psk: The new PSK
	 * @psk_len: The length of psk in octets
	 * Returns: 0 on success, -1 on failure
	 *
	 * This callback is called when a new per-device PSK is provisioned.
	 */
	int (*new_psk_cb)(void *ctx, const u8 *mac_addr, const u8 *psk,
			  size_t psk_len);

	/**
	 * set_ie_cb - Callback for WPS IE changes
	 * @ctx: Higher layer context data (cb_ctx)
	 * @beacon_ie: WPS IE for Beacon
	 * @beacon_ie_len: WPS IE length for Beacon
	 * @probe_resp_ie: WPS IE for Probe Response
	 * @probe_resp_ie_len: WPS IE length for Probe Response
	 * Returns: 0 on success, -1 on failure
	 *
	 * This callback is called whenever the WPS IE in Beacon or Probe
	 * Response frames needs to be changed (AP only).
	 */
	int (*set_ie_cb)(void *ctx, const u8 *beacon_ie, size_t beacon_ie_len,
			 const u8 *probe_resp_ie, size_t probe_resp_ie_len);

	/**
	 * pin_needed_cb - Callback for requesting a PIN
	 * @ctx: Higher layer context data (cb_ctx)
	 * @uuid_e: UUID-E of the unknown Enrollee
	 * @dev: Device Data from the unknown Enrollee
	 *
	 * This callback is called whenever an unknown Enrollee requests to use
	 * PIN method and a matching PIN (Device Password) is not found in
	 * Registrar data.
	 */
	void (*pin_needed_cb)(void *ctx, const u8 *uuid_e,
			      const struct wps_device_data *dev);

	/**
	 * reg_success_cb - Callback for reporting successful registration
	 * @ctx: Higher layer context data (cb_ctx)
	 * @mac_addr: MAC address of the Enrollee
	 * @uuid_e: UUID-E of the Enrollee
	 *
	 * This callback is called whenever an Enrollee completes registration
	 * successfully.
	 */
	void (*reg_success_cb)(void *ctx, const u8 *mac_addr,
			       const u8 *uuid_e);

	/**
	 * cb_ctx: Higher layer context data for Registrar callbacks
	 */
	void *cb_ctx;

	/**
	 * skip_cred_build: Do not build credential
	 *
	 * This option can be used to disable internal code that builds
	 * Credential attribute into M8 based on the current network
	 * configuration and Enrollee capabilities. The extra_cred data will
	 * then be used as the Credential(s).
	 */
	int skip_cred_build;

	/**
	 * extra_cred: Additional Credential attribute(s)
	 *
	 * This optional data (set to %NULL to disable) can be used to add
	 * Credential attribute(s) for other networks into M8. If
	 * skip_cred_build is set, this will also override the automatically
	 * generated Credential attribute.
	 */
	const u8 *extra_cred;

	/**
	 * extra_cred_len: Length of extra_cred in octets
	 */
	size_t extra_cred_len;

	/**
	 * disable_auto_conf - Disable auto-configuration on first registration
	 *
	 * By default, the AP that is started in not configured state will
	 * generate a random PSK and move to configured state when the first
	 * registration protocol run is completed successfully. This option can
	 * be used to disable this functionality and leave it up to an external
	 * program to take care of configuration. This requires the extra_cred
	 * to be set with a suitable Credential and skip_cred_build being used.
	 */
	int disable_auto_conf;

	/**
	 * static_wep_only - Whether the BSS supports only static WEP
	 */
	int static_wep_only;
};


enum wps_event {
	/**
	 * WPS_EV_M2D - M2D received (Registrar did not know us)
	 */
	WPS_EV_M2D,

	/**
	 * WPS_EV_FAIL - Registration failed
	 */
	WPS_EV_FAIL,

	/**
	 * WPS_EV_SUCCESS - Registration succeeded
	 */
	WPS_EV_SUCCESS,

	/**
	 * WPS_EV_PWD_AUTH_FAIL - Password authentication failed
	 */
	WPS_EV_PWD_AUTH_FAIL,

	/**
	 * WPS_EV_PBC_OVERLAP - PBC session overlap detected
	 */
	WPS_EV_PBC_OVERLAP,

	/**
	 * WPS_EV_PBC_TIMEOUT - PBC walktime expired before protocol run start
	 */
	WPS_EV_PBC_TIMEOUT
};

union wps_event_data {
	/**
	 * struct wps_event_m2d - M2D event data
	 */
	struct wps_event_m2d {
		u16 config_methods;
		const u8 *manufacturer;
		size_t manufacturer_len;
		const u8 *model_name;
		size_t model_name_len;
		const u8 *model_number;
		size_t model_number_len;
		const u8 *serial_number;
		size_t serial_number_len;
		const u8 *dev_name;
		size_t dev_name_len;
		const u8 *primary_dev_type; /* 8 octets */
		u16 config_error;
		u16 dev_password_id;
	} m2d;

	/**
	 * struct wps_event_fail - Registration failure information
	 * @msg: enum wps_msg_type
	 */
	struct wps_event_fail {
		int msg;
		u16 config_error;
	} fail;

	struct wps_event_pwd_auth_fail {
		int enrollee;
		int part;
	} pwd_auth_fail;
};

struct upnp_pending_message {
	struct upnp_pending_message *next;
	u8 addr[ETH_ALEN];
	struct wpabuf *msg;
	enum wps_msg_type type;
};

struct wps_context {
	/**
	 * ap - Whether the local end is an access point
	 */
	int ap;

	/**
	 * registrar - Pointer to WPS registrar data from wps_registrar_init()
	 */
	struct wps_registrar *registrar;

	/**
	 * wps_state - Current WPS state
	 */
	enum wps_state wps_state;

	/**
	 * ap_setup_locked - Whether AP setup is locked (only used at AP)
	 */
	int ap_setup_locked;

	/**
	 * uuid - Own UUID
	 */
	u8 uuid[16];

	/**
	 * ssid - SSID
	 *
	 * This SSID is used by the Registrar to fill in information for
	 * Credentials. In addition, AP uses it when acting as an Enrollee to
	 * notify Registrar of the current configuration.
	 */
	u8 ssid[32];

	/**
	 * ssid_len - Length of ssid in octets
	 */
	size_t ssid_len;

	/**
	 * dev - Own WPS device data
	 */
	struct wps_device_data dev;

	/**
	 * config_methods - Enabled configuration methods
	 *
	 * Bit field of WPS_CONFIG_*
	 */
	u16 config_methods;

	/**
	 * encr_types - Enabled encryption types (bit field of WPS_ENCR_*)
	 */
	u16 encr_types;

	/**
	 * auth_types - Authentication types (bit field of WPS_AUTH_*)
	 */
	u16 auth_types;

	/**
	 * network_key - The current Network Key (PSK) or %NULL to generate new
	 *
	 * If %NULL, Registrar will generate per-device PSK. In addition, AP
	 * uses this when acting as an Enrollee to notify Registrar of the
	 * current configuration.
	 */
	u8 *network_key;

	/**
	 * network_key_len - Length of network_key in octets
	 */
	size_t network_key_len;

	/**
	 * ap_settings - AP Settings override for M7 (only used at AP)
	 *
	 * If %NULL, AP Settings attributes will be generated based on the
	 * current network configuration.
	 */
	u8 *ap_settings;

	/**
	 * ap_settings_len - Length of ap_settings in octets
	 */
	size_t ap_settings_len;

	/**
	 * friendly_name - Friendly Name (required for UPnP)
	 */
	char *friendly_name;

	/**
	 * manufacturer_url - Manufacturer URL (optional for UPnP)
	 */
	char *manufacturer_url;

	/**
	 * model_description - Model Description (recommended for UPnP)
	 */
	char *model_description;

	/**
	 * model_url - Model URL (optional for UPnP)
	 */
	char *model_url;

	/**
	 * upc - Universal Product Code (optional for UPnP)
	 */
	char *upc;

	/**
	 * cred_cb - Callback to notify that new Credentials were received
	 * @ctx: Higher layer context data (cb_ctx)
	 * @cred: The received Credential
	 * Return: 0 on success, -1 on failure
	 */
	int (*cred_cb)(void *ctx, const struct wps_credential *cred);

	/**
	 * event_cb - Event callback (state information about progress)
	 * @ctx: Higher layer context data (cb_ctx)
	 * @event: Event type
	 * @data: Event data
	 */
	void (*event_cb)(void *ctx, enum wps_event event,
			 union wps_event_data *data);

	/**
	 * cb_ctx: Higher layer context data for callbacks
	 */
	void *cb_ctx;

	struct upnp_wps_device_sm *wps_upnp;

	/* Pending messages from UPnP PutWLANResponse */
	struct upnp_pending_message *upnp_msgs;
};


struct wps_registrar *
wps_registrar_init(struct wps_context *wps,
		   const struct wps_registrar_config *cfg);
void wps_registrar_deinit(struct wps_registrar *reg);
int wps_registrar_add_pin(struct wps_registrar *reg, const u8 *uuid,
			  const u8 *pin, size_t pin_len, int timeout);
int wps_registrar_invalidate_pin(struct wps_registrar *reg, const u8 *uuid);
int wps_registrar_unlock_pin(struct wps_registrar *reg, const u8 *uuid);
int wps_registrar_button_pushed(struct wps_registrar *reg);
void wps_registrar_probe_req_rx(struct wps_registrar *reg, const u8 *addr,
				const struct wpabuf *wps_data);
int wps_registrar_update_ie(struct wps_registrar *reg);
int wps_registrar_set_selected_registrar(struct wps_registrar *reg,
					 const struct wpabuf *msg);

unsigned int wps_pin_checksum(unsigned int pin);
unsigned int wps_pin_valid(unsigned int pin);
unsigned int wps_generate_pin(void);
void wps_free_pending_msgs(struct upnp_pending_message *msgs);

u16 wps_config_methods_str2bin(const char *str);


#ifdef CONFIG_WPS_STRICT
int wps_validate_beacon(const struct wpabuf *wps_ie);
int wps_validate_beacon_probe_resp(const struct wpabuf *wps_ie, int probe,
				   const u8 *addr);
int wps_validate_probe_req(const struct wpabuf *wps_ie, const u8 *addr);
int wps_validate_assoc_req(const struct wpabuf *wps_ie);
int wps_validate_assoc_resp(const struct wpabuf *wps_ie);
int wps_validate_m1(const struct wpabuf *tlvs);
int wps_validate_m2(const struct wpabuf *tlvs);
int wps_validate_m2d(const struct wpabuf *tlvs);
int wps_validate_m3(const struct wpabuf *tlvs);
int wps_validate_m4(const struct wpabuf *tlvs);
int wps_validate_m4_encr(const struct wpabuf *tlvs, int wps2);
int wps_validate_m5(const struct wpabuf *tlvs);
int wps_validate_m5_encr(const struct wpabuf *tlvs, int wps2);
int wps_validate_m6(const struct wpabuf *tlvs);
int wps_validate_m6_encr(const struct wpabuf *tlvs, int wps2);
int wps_validate_m7(const struct wpabuf *tlvs);
int wps_validate_m7_encr(const struct wpabuf *tlvs, int ap, int wps2);
int wps_validate_m8(const struct wpabuf *tlvs);
int wps_validate_m8_encr(const struct wpabuf *tlvs, int ap, int wps2);
int wps_validate_wsc_ack(const struct wpabuf *tlvs);
int wps_validate_wsc_nack(const struct wpabuf *tlvs);
int wps_validate_wsc_done(const struct wpabuf *tlvs);
int wps_validate_upnp_set_selected_registrar(const struct wpabuf *tlvs);
#else /* CONFIG_WPS_STRICT */
static inline int wps_validate_beacon(const struct wpabuf *wps_ie){
	return 0;
}

static inline int wps_validate_beacon_probe_resp(const struct wpabuf *wps_ie,
						 int probe, const u8 *addr)
{
	return 0;
}

static inline int wps_validate_probe_req(const struct wpabuf *wps_ie,
					 const u8 *addr)
{
	return 0;
}

static inline int wps_validate_assoc_req(const struct wpabuf *wps_ie)
{
	return 0;
}

static inline int wps_validate_assoc_resp(const struct wpabuf *wps_ie)
{
	return 0;
}

static inline int wps_validate_m1(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m2(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m2d(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m3(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m4(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m4_encr(const struct wpabuf *tlvs, int wps2)
{
	return 0;
}

static inline int wps_validate_m5(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m5_encr(const struct wpabuf *tlvs, int wps2)
{
	return 0;
}

static inline int wps_validate_m6(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m6_encr(const struct wpabuf *tlvs, int wps2)
{
	return 0;
}

static inline int wps_validate_m7(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m7_encr(const struct wpabuf *tlvs, int ap,
				       int wps2)
{
	return 0;
}

static inline int wps_validate_m8(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_m8_encr(const struct wpabuf *tlvs, int ap,
				       int wps2)
{
	return 0;
}

static inline int wps_validate_wsc_ack(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_wsc_nack(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_wsc_done(const struct wpabuf *tlvs)
{
	return 0;
}

static inline int wps_validate_upnp_set_selected_registrar(
	const struct wpabuf *tlvs)
{
	return 0;
}
#endif /* CONFIG_WPS_STRICT */


#endif /* WPS_H */
