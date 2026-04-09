#include "maptool.h"
#include "transform.h"
#include <gdal.h>
#include <ogr_core.h>

void process_coastlines(char *in, FILE *out) {
    // 1. Osmcoastline über System-Call ausführen
    // -o definiert die Ausgabe-Datenbank
    char *suffix = "db";
    rename(in, g_strdup_printf("%s/%s_%s.osm", tempfile_obtain_prefix(), "osmcoastline", suffix));
    char *output_db = tempfile_name(suffix, "osmcoastline");

    char command[512];
    snprintf(command, sizeof(command), "osmcoastline --output-polygons=water -v -o %s %s", output_db,
             g_strdup_printf("%s/%s_%s.osm", tempfile_obtain_prefix(), "osmcoastline", suffix));

    printf("Starte osmcoastline...\n");
    int ret = system(command);

    if (ret != 0) {
        fprintf(stderr, "Fehler: osmcoastline fehlgeschlagen (Exit-Code %d).\n", ret);
    }
    printf("Prozessierung abgeschlossen. Öffne Datenbank...\n");

    // 2. GDAL/OGR Initialisierung & Datei öffnen
    GDALAllRegister();
    GDALDatasetH hDS = GDALOpenEx(output_db, GDAL_OF_VECTOR, NULL, NULL, NULL);

    if (hDS == NULL) {
        fprintf(stderr, "Fehler: OGR konnte %s nicht öffnen.\n", output_db);
    }

    // 3. Zugriff auf die generierten Layer (z.B. water_polygons)
    OGRLayerH hLayer = GDALDatasetGetLayerByName(hDS, "water_polygons");
    if (hLayer == NULL) {
        printf("Layer 'water_polygons' nicht erfolgreich geladen.");
    }
    OGR_L_ResetReading(hLayer);
    OGRFeatureH hFeature;
    while ((hFeature = OGR_L_GetNextFeature(hLayer)) != NULL) {
        OGRGeometryH hGeom = OGR_F_GetGeometryRef(hFeature);
        /*int nSubGeoms = OGR_G_GetGeometryCount(hGeom);
        struct item_bin *ib;
        for (int j = 0; j < nSubGeoms; j++) {

            OGRGeometryH hSubGeom = OGR_G_GetGeometryRef(hGeom, j);

            if (hSubGeom && OGR_G_GetGeometryType(hSubGeom) == wkbPolygon) {
                OGRGeometryH hRing = OGR_G_GetGeometryRef(hSubGeom, 0);
                int nPoints = OGR_G_GetPointCount(hRing);
                ib = init_item(type_poly_water_tiled);
                struct coord_geo c_geo[nPoints];
                struct coord c[nPoints];
                for (int i = 1; i < nPoints; i++) {

                    OGR_G_GetPoint(hRing, i, &c_geo[i].lng, &c_geo[i].lat, NULL);
                    //c.x = x * 6371000.0 * G_PI / 180;
                    //c.y = log(tan(G_PI_4 + y * G_PI / 360)) * 6371000.0;
                    transform_from_geo(projection_mg, &c_geo[i], &c[i]);
                }
                if(j == 0){
                    // Erster Ring ist Außenring, weitere Ringe sind Löcher
                    item_bin_add_coord_reverse(ib, c, nPoints);
                } else {
                    // Löcher in umgekehrter Reihenfolge hinzufügen
                    item_bin_add_hole(ib, c, nPoints);
                }
            }

        }
        item_bin_write(ib, out);
        OGR_F_Destroy(hFeature);
    }*/
        OGRwkbGeometryType type = wkbFlatten(OGR_G_GetGeometryType(hGeom));

        if (type == wkbPolygon) {
            // Direkt Polygon behandeln
            struct item_bin *ib = init_item(type_poly_water_tiled);

            int ringCount = OGR_G_GetGeometryCount(hGeom);

            for (int j = 0; j < ringCount; j++) {
                OGRGeometryH hRing = OGR_G_GetGeometryRef(hGeom, j);

                int nPoints = OGR_G_GetPointCount(hRing);
                struct coord_geo c_geo[nPoints];
                struct coord c[nPoints];

                for (int i = 0; i < nPoints; i++) {
                    OGR_G_GetPoint(hRing, i, &c_geo[i].lng, &c_geo[i].lat, NULL);
                    transform_from_geo(projection_mg, &c_geo[i], &c[i]);
                }

                if (j == 0) {
                    item_bin_add_coord_reverse(ib, c, nPoints);  // Außenring
                } else {
                    item_bin_add_hole(ib, c, nPoints);  // Loch
                }
            }

            item_bin_write(ib, out);
        } else if (type == wkbMultiPolygon) {
            int nPolys = OGR_G_GetGeometryCount(hGeom);

            for (int p = 0; p < nPolys; p++) {
                OGRGeometryH hPoly = OGR_G_GetGeometryRef(hGeom, p);

                struct item_bin *ib = init_item(type_poly_water_tiled);

                int ringCount = OGR_G_GetGeometryCount(hPoly);

                for (int j = 0; j < ringCount; j++) {
                    OGRGeometryH hRing = OGR_G_GetGeometryRef(hPoly, j);

                    int nPoints = OGR_G_GetPointCount(hRing);
                    struct coord_geo c_geo[nPoints];
                    struct coord c[nPoints];

                    for (int i = 0; i < nPoints; i++) {
                        OGR_G_GetPoint(hRing, i, &c_geo[i].lng, &c_geo[i].lat, NULL);
                        transform_from_geo(projection_mg, &c_geo[i], &c[i]);
                    }

                    if (j == 0) {
                        item_bin_add_coord_reverse(ib, c, nPoints);
                    } else {
                        item_bin_add_hole(ib, c, nPoints);
                    }
                }

                item_bin_write(ib, out);
            }
        }
    }
    GDALClose(hDS);
}
