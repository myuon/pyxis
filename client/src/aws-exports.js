import Amplify from 'aws-amplify';

console.log('configuring...');

Amplify.configure({
  Auth: {
    identityPoolId: 'ap-northeast-1:83240432-38c1-4654-83ef-08fe12e3be63',
    region: 'ap-northeast-1', 
    userPoolId: 'ap-northeast-1_GOhWfmkaq', 
    userPoolWebClientId: '52di0k76qmf4lpduodf1rg9ttp',
    authenticationFlowType: 'USER_PASSWORD_AUTH',
  },
});
