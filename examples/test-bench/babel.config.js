module.exports = function (api) {
  api.cache(true);
  return {
    presets: ['babel-preset-expo'],
    plugins: [
      // Must be listed last per react-native-worklets docs.
      'react-native-worklets/plugin',
    ],
  };
};
