// Skia 3.x needs Android minSdkVersion 26; Expo's generated app defaults to 24.
const { withGradleProperties } = require('expo/config-plugins');

module.exports = (config) =>
  withGradleProperties(config, (cfg) => {
    cfg.modResults = cfg.modResults.filter(
      (item) => !(item.type === 'property' && item.key === 'android.minSdkVersion'),
    );
    cfg.modResults.push({ type: 'property', key: 'android.minSdkVersion', value: '26' });
    return cfg;
  });
