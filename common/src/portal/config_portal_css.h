#ifndef CONFIG_PORTAL_CSS_H
#define CONFIG_PORTAL_CSS_H

const char CONFIG_PORTAL_CSS[] PROGMEM = R"(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 20px;
}

.container {
    background: white;
    border-radius: 12px;
    box-shadow: 0 10px 40px rgba(0,0,0,0.2);
    padding: 40px;
    max-width: 600px;
    width: 100%;
}

h1 {
    color: #333;
    margin-bottom: 10px;
    font-size: 28px;
}

h2 {
    color: #555;
    margin-bottom: 15px;
    margin-top: 25px;
    font-size: 20px;
    border-bottom: 2px solid #667eea;
    padding-bottom: 8px;
}

.device-id {
    color: #888;
    margin-bottom: 30px;
    font-size: 14px;
}

.section {
    margin-bottom: 25px;
}

label {
    display: block;
    margin-bottom: 8px;
    color: #555;
    font-weight: 500;
}

input[type="text"],
input[type="password"] {
    width: 100%;
    padding: 12px;
    border: 2px solid #e0e0e0;
    border-radius: 6px;
    font-size: 14px;
    margin-bottom: 15px;
    transition: border-color 0.3s;
}

input[type="text"]:focus,
input[type="password"]:focus {
    outline: none;
    border-color: #667eea;
}

input[type="checkbox"] {
    margin-right: 8px;
    width: 18px;
    height: 18px;
    vertical-align: middle;
}

.btn-primary {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
    border: none;
    padding: 14px 30px;
    border-radius: 6px;
    font-size: 16px;
    font-weight: 600;
    cursor: pointer;
    width: 100%;
    transition: transform 0.2s, box-shadow 0.2s;
}

.btn-primary:hover {
    transform: translateY(-2px);
    box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
}

.btn-primary:active {
    transform: translateY(0);
}

.error {
    color: #e74c3c;
    background: #fadbd8;
    padding: 15px;
    border-radius: 6px;
    margin-bottom: 20px;
}

a {
    color: #667eea;
    text-decoration: none;
    font-weight: 500;
}

a:hover {
    text-decoration: underline;
}

.btn-secondary {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
    padding: 12px 24px;
    border: none;
    border-radius: 6px;
    font-size: 16px;
    cursor: pointer;
    transition: transform 0.2s;
}

.btn-secondary:hover {
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.btn-danger {
    background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
    color: white;
    padding: 12px 24px;
    border: none;
    border-radius: 6px;
    font-size: 16px;
    cursor: pointer;
    transition: transform 0.2s;
}

.btn-danger:hover {
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(245, 87, 108, 0.4);
}

@media (max-width: 600px) {
    .container {
        padding: 20px;
    }
    
    h1 {
        font-size: 24px;
    }
    
    h2 {
        font-size: 18px;
    }
}
)";

#endif // CONFIG_PORTAL_CSS_H
