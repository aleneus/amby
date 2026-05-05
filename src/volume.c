#include "volume.h"

#include <sensors/sensors.h>

#include "mixer.h"
#include "threads.h"

static float
calculate_gain (float temp)
{
  static const float min_temp = 35.0f;
  static const float max_temp = 100.0f;

  float v = (temp - min_temp) / (max_temp - min_temp);
  return (v < 0.0f) ? 0.0f : (v > 0.3f ? 0.3f : v);
}

static void
find_thermal_sensor (const sensors_chip_name **found_chip,
                     const sensors_subfeature **found_sf)
{
  const sensors_chip_name *chip;
  int chip_nr = 0;

  *found_chip = NULL;
  *found_sf = NULL;

  while ((chip = sensors_get_detected_chips (NULL, &chip_nr)) != NULL)
    {
      int feat_nr = 0;
      const sensors_feature *feat;

      while ((feat = sensors_get_features (chip, &feat_nr)) != NULL)
        {
          if (feat->type == SENSORS_FEATURE_TEMP)
            {
              *found_sf = sensors_get_subfeature (
                  chip, feat, SENSORS_SUBFEATURE_TEMP_INPUT);

              if (*found_sf != NULL)
                {
                  *found_chip = chip;
                  return;
                }
            }
        }
    }
}

void *
volume_controller_thread (void *arg)
{
  mixer_t *mx = (mixer_t *)arg;
  const sensors_chip_name *found = NULL;
  const sensors_subfeature *sf = NULL;

  if (sensors_init (NULL) != 0)
    {
      return NULL;
    }

  find_thermal_sensor (&found, &sf);

  while (keep_running)
    {
      double val;

      if (found && sf && sensors_get_value (found, sf->number, &val) == 0)
        {
          float volume = calculate_gain ((float)val);
          mixer_set_gain (mx, volume);
        }

      usleep (200000);
    }

  sensors_cleanup ();

  return NULL;
}
