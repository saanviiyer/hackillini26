const crypto = require('crypto');
const jwt = require('jsonwebtoken');
const fs = require('fs');

// 1. Your Account and Username (Must be UPPERCASE)
const account = 'IOSANKQ-HK55137'.toUpperCase();
const user = 'SAANVISUB'.toUpperCase();

// 2. Read the private and public keys
const privateKey = fs.readFileSync('rsa_key.p8', 'utf8');
const publicKey = fs.readFileSync('rsa_key.pub', 'utf8');

// 3. Snowflake requires a special "fingerprint" of the public key
const publicKeyDer = crypto.createPublicKey(publicKey).export({ format: 'der', type: 'spki' });
const fingerprint = 'SHA256:' + crypto.createHash('sha256').update(publicKeyDer).digest('base64');

// 4. Build the payload
const payload = {
    iss: `${account}.${user}.${fingerprint}`,
    sub: `${account}.${user}`,
    iat: Math.floor(Date.now() / 1000),
    exp: Math.floor(Date.now() / 1000) + 21600 // 21600 seconds = 6 hours
};

// 5. Generate and print the token!
const token = jwt.sign(payload, privateKey, { algorithm: 'RS256' });
console.log('\n=== COPY THIS INTO YOUR .dev.vars ===\n');
console.log(`SNOWFLAKE_ACCOUNT="IOSANKQ-HK55137"`);
console.log(`SNOWFLAKE_TOKEN="${token}"`);
console.log('\n=====================================\n');