/*
 * walky-hmi-controller.c
 *
 * Contrôleur ivi-shell minimal pour Walky.
 * Remplace hmi-controller.so (référence Weston) par une implémentation
 * réduite à 2 layers fixes :
 *
 *   - WALKY_LAYER_APP     (z-order bas)  : app plein écran (menu QML, Spotify, Deezer)
 *   - WALKY_LAYER_OVERLAY (z-order haut) : clavier virtuel / bandeau / indicateurs
 *
 * Vérifié contre /usr/include/weston/ivi-layout-export.h (Weston 13.0.1,
 * Scarthgap). Points encore ouverts marqués TODO.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libweston/libweston.h>
#include <weston/ivi-layout-export.h>

#define WALKY_LAYER_APP_ID      1000
#define WALKY_LAYER_OVERLAY_ID  2000

/* IDs de surface ivi_application attendus des clients "maison" (menu QML,
 * clavier virtuel). Doivent être envoyés par les clients Wayland eux-mêmes
 * via le protocole ivi_application (ivi_id).
 * TODO: fixer la convention d'ID définitive et la documenter côté client Qt. */
#define WALKY_SURFACE_ID_MENU      100
#define WALKY_SURFACE_ID_KEYBOARD  200
/* TODO: Spotify/Deezer (clients tiers, xdg-shell) n'enverront pas d'ivi_id
 * volontaire — identification à faire autrement (app-id xdg-shell / nom de
 * process). Non couvert dans ce squelette. */

struct walky_hmi {
    struct weston_compositor *compositor;
    const struct ivi_layout_interface *api;

    struct weston_output *output;

    struct ivi_layout_layer *layer_app;
    struct ivi_layout_layer *layer_overlay;

    struct wl_listener surface_created_listener;
    struct wl_listener surface_removed_listener;
};

/* Route une nouvelle surface vers le bon layer selon son ivi_id, et
 * retourne le rectangle de destination associé.
 *
 * Confirmé fonctionnel visuellement le 03/09/2026 : deux layers
 * simultanément affichés (app plein écran + overlay réduit en haut à
 * droite), contrairement à kiosk-shell qui ne montrait qu'une seule
 * surface à la fois (cf. walky-overlay-weston-a-approfondir.md).
 *
 * TODO: les clients xdg-shell sans ivi_id (dont weston-terminal utilisé
 * pour ce test, et futurs clients tiers type Spotify) obtiennent tous
 * ivi_id == IVI_INVALID_ID (0xFFFFFFFF) — ils tombent donc actuellement
 * dans la branche par défaut (app). Spotify/Deezer devront être
 * identifiés autrement (app-id xdg-shell / nom de process), non couvert
 * ici — cf. TODO plus bas dans wet_module_init. */
static struct ivi_layout_layer *
walky_pick_layer_for_surface(struct walky_hmi *hmi, uint32_t ivi_id,
                              int32_t *dst_x, int32_t *dst_y,
                              int32_t *dst_w, int32_t *dst_h)
{
    switch (ivi_id) {
    case WALKY_SURFACE_ID_KEYBOARD:
        /* TODO: dimensions/position provisoires (tiers d'écran, coin
         * haut-droit) — à ajuster une fois la maquette réelle du clavier
         * virtuel définie (cf. walky-interfaces.md, section 3). */
        *dst_w = hmi->output->width / 3;
        *dst_h = hmi->output->height / 3;
        *dst_x = hmi->output->width - *dst_w;
        *dst_y = 0;
        return hmi->layer_overlay;

    case WALKY_SURFACE_ID_MENU:
    default:
        /* Par défaut, tout ce qu'on ne reconnaît pas va dans le layer app
         * en plein écran (menu, Spotify, Deezer, et pour l'instant tout
         * client xdg-shell sans ivi_id).
         * TODO: whitelist plutôt que ce défaut permissif, une fois
         * Spotify identifié précisément côté xdg-shell. */
        *dst_x = 0;
        *dst_y = 0;
        *dst_w = hmi->output->width;
        *dst_h = hmi->output->height;
        return hmi->layer_app;
    }
}

static void
walky_handle_surface_create(struct wl_listener *listener, void *data)
{
    struct ivi_layout_surface *ivi_surf = data;
    struct walky_hmi *hmi =
        wl_container_of(listener, hmi, surface_created_listener);

    uint32_t ivi_id = hmi->api->get_id_of_surface(ivi_surf);

    int32_t dst_x, dst_y, dst_w, dst_h;
    struct ivi_layout_layer *target_layer =
        walky_pick_layer_for_surface(hmi, ivi_id, &dst_x, &dst_y, &dst_w, &dst_h);

    weston_log("walky-hmi-controller: nouvelle surface ivi_id=%u "
               "-> layer=%s rect=%dx%d+%d+%d\n",
               ivi_id,
               (target_layer == hmi->layer_overlay) ? "overlay" : "app",
               dst_w, dst_h, dst_x, dst_y);

    hmi->api->layer_add_surface(target_layer, ivi_surf);


    hmi->api->surface_set_source_rectangle(
        ivi_surf, 0, 0, hmi->output->width, hmi->output->height);

    hmi->api->surface_set_destination_rectangle(
        ivi_surf, dst_x, dst_y, dst_w, dst_h);

    hmi->api->surface_set_visibility(ivi_surf, true);

    hmi->api->commit_changes();

    weston_log("walky-hmi-controller: commit fait pour ivi_id=%u\n", ivi_id);
}

static void
walky_handle_surface_remove(struct wl_listener *listener, void *data)
{
    /* TODO: nettoyage d'état interne si besoin (ex: remettre le layer
     * overlay en attente quand le clavier se ferme). Rien de nécessaire
     * pour l'instant, ivi-shell gère déjà le retrait de la surface. */
    (void) listener;
    (void) data;
}

static struct weston_output *
walky_get_first_output(struct weston_compositor *compositor)
{
    struct weston_output *output;

    wl_list_for_each(output, &compositor->output_list, link) {
        return output;
        /* TODO: pour un jour multi-écran, il faudrait choisir le bon
         * output plutôt que prendre le premier. Walky = un seul écran,
         * donc suffisant pour l'instant. */
    }

    return NULL;
}

static struct walky_hmi *
walky_hmi_create(struct weston_compositor *compositor,
                  const struct ivi_layout_interface *api)
{
    struct walky_hmi *hmi = zalloc(sizeof(*hmi));
    if (!hmi)
        return NULL;

    hmi->compositor = compositor;
    hmi->api = api;

    hmi->output = walky_get_first_output(compositor);
    if (!hmi->output) {
        weston_log("walky-hmi-controller: aucun weston_output trouvé\n");
        free(hmi);
        return NULL;
    }

    int32_t width  = hmi->output->width;
    int32_t height = hmi->output->height;

    hmi->layer_app = api->layer_create_with_dimension(
        WALKY_LAYER_APP_ID, width, height);
    hmi->layer_overlay = api->layer_create_with_dimension(
        WALKY_LAYER_OVERLAY_ID, width, height);

    if (!hmi->layer_app || !hmi->layer_overlay) {
        weston_log("walky-hmi-controller: échec création des layers\n");
        free(hmi);
        return NULL;
    }

    api->layer_set_destination_rectangle(hmi->layer_app, 0, 0, width, height);
    api->layer_set_destination_rectangle(hmi->layer_overlay, 0, 0, width, height);

    api->layer_set_visibility(hmi->layer_app, true);
    api->layer_set_visibility(hmi->layer_overlay, true);

    api->screen_add_layer(hmi->output, hmi->layer_app);
    api->screen_add_layer(hmi->output, hmi->layer_overlay);

    /* Ordre de rendu explicite : overlay au-dessus de app.
     * Confirmé par test visuel le 03/09/2026 : [app, overlay] donne bien
     * overlay au premier plan (index croissant = plus proche du premier
     * plan dans screen_set_render_order). */
    struct ivi_layout_layer *render_order[2] = {
        hmi->layer_app,
        hmi->layer_overlay,
    };
    api->screen_set_render_order(hmi->output, render_order, 2);

    hmi->surface_created_listener.notify = walky_handle_surface_create;
    api->add_listener_create_surface(&hmi->surface_created_listener);

    hmi->surface_removed_listener.notify = walky_handle_surface_remove;
    api->add_listener_remove_surface(&hmi->surface_removed_listener);

    api->commit_changes();

    return hmi;
}

/* Point d'entrée du module, chargé par Weston via
 * weston.ini: modules=walky-hmi-controller.so */
WL_EXPORT int
wet_module_init(struct weston_compositor *compositor,
                 int *argc, char *argv[])
{
    (void) argc;
    (void) argv;

    const struct ivi_layout_interface *api = ivi_layout_get_api(compositor);
    if (!api) {
        weston_log("walky-hmi-controller: impossible de récupérer "
                   "ivi_layout_interface — ivi-shell.so est-il bien "
                   "chargé avant ce module dans weston.ini ?\n");
        return -1;
    }

    struct walky_hmi *hmi = walky_hmi_create(compositor, api);
    if (!hmi) {
        weston_log("walky-hmi-controller: échec de l'initialisation\n");
        return -1;
    }

    weston_log("walky-hmi-controller: initialisé (layer app=%d, overlay=%d)\n",
               WALKY_LAYER_APP_ID, WALKY_LAYER_OVERLAY_ID);

    return 0;
}