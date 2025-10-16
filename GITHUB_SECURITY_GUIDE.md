# GitHub Upload Security Guide

## 🔐 CRITICAL: Before You Upload to GitHub

### ⚠️ What NOT to Upload

**NEVER commit these sensitive items:**
- ❌ Device tokens/passwords
- ❌ API keys
- ❌ Server IP addresses (if private)
- ❌ WiFi credentials
- ❌ MQTT passwords
- ❌ Any authentication secrets

## ✅ Safe Upload Checklist

### Step 1: Prepare Files

```bash
# Your repository structure should look like:
mushroom-house-controller/
├── mushroom_controller_github.ino  ← SAFE (uses config.h)
├── config_template.h               ← SAFE (template only)
├── config.h                        ← NOT IN GIT (your actual credentials)
├── .gitignore                      ← SAFE (protects config.h)
├── README.md                       ← SAFE
├── FEATURE_DOCUMENTATION.md        ← SAFE
├── DASHBOARD_SETUP_GUIDE.md        ← SAFE
└── LICENSE                         ← SAFE
```

### Step 2: Verify .gitignore

Your `.gitignore` should contain:
```
config.h
credentials.h
secrets.h
```

### Step 3: Check What's Being Committed

Before `git add`:
```bash
# Review what will be committed
git status

# Make sure config.h is NOT listed
# If it appears, check your .gitignore
```

### Step 4: Verify No Secrets

```bash
# Search for potential secrets in files to be committed
grep -r "TOKEN" *.ino
grep -r "PASSWORD" *.ino
grep -r "const char\* TOKEN" *.ino

# Should find NOTHING in mushroom_controller_github.ino
# Only in config.h (which is gitignored)
```

## 📋 Step-by-Step Upload Process

### First Time Setup

```bash
# 1. Create GitHub repository (on github.com)
#    - Go to github.com
#    - Click "New repository"
#    - Name: "mushroom-house-controller"
#    - Keep it PUBLIC or PRIVATE (your choice)
#    - Don't initialize with README (we have our own)

# 2. In your local project folder:
cd /path/to/your/project

# 3. Initialize git (if not already done)
git init

# 4. Copy safe files from outputs
cp mushroom_controller_github.ino mushroom_controller.ino
cp config_template.h .
cp .gitignore .
cp README.md .
cp FEATURE_DOCUMENTATION.md .
cp DASHBOARD_SETUP_GUIDE.md .

# 5. Create your actual config.h (NOT in git)
cp config_template.h config.h
nano config.h  # Add your real credentials

# 6. VERIFY config.h is gitignored
git status
# Should NOT see config.h in the list

# 7. Add files
git add .

# 8. Verify again what's being added
git status
# config.h should still NOT appear

# 9. Commit
git commit -m "Initial commit - Mushroom House Controller v2.0"

# 10. Add remote
git remote add origin https://github.com/YOUR_USERNAME/mushroom-house-controller.git

# 11. Push
git push -u origin main
# (or 'master' depending on your default branch)
```

## 🚨 What If You Already Exposed Secrets?

### Option 1: Repository Never Pushed (Easy)

```bash
# Just remove the file and recommit
git rm --cached config.h
git commit -m "Remove sensitive config file"
```

### Option 2: Already Pushed to GitHub (More Work)

```bash
# 1. IMMEDIATELY regenerate all exposed credentials
#    - Create new ThingsBoard device token
#    - Change any exposed passwords

# 2. Remove from Git history
git filter-branch --force --index-filter \
  "git rm --cached --ignore-unmatch config.h" \
  --prune-empty --tag-name-filter cat -- --all

# 3. Force push (overwrites remote)
git push origin --force --all
git push origin --force --tags

# 4. Update config.h with NEW credentials
nano config.h

# 5. Verify .gitignore is working
git status  # config.h should NOT appear
```

### Option 3: Nuclear Option (Start Fresh)

```bash
# 1. Delete GitHub repository (on github.com)

# 2. Regenerate ALL credentials

# 3. Create new repository

# 4. Follow "First Time Setup" above with new credentials
```

## 🔍 Verification After Upload

### Check Your GitHub Repository

1. Go to your repository on github.com
2. Browse files - should see:
   - ✅ mushroom_controller.ino
   - ✅ config_template.h
   - ✅ README.md
   - ✅ .gitignore
   - ❌ config.h (should NOT be here!)

### Search for Exposed Secrets

On GitHub, use search:
```
repo:YOUR_USERNAME/mushroom-house-controller TOKEN
repo:YOUR_USERNAME/mushroom-house-controller password
repo:YOUR_USERNAME/mushroom-house-controller 167.99.69
```

Should return NO results!

## 🎯 Best Practices

### 1. Use Environment Variables (Advanced)

For even better security:

```cpp
// Instead of config.h, use environment variables
const char* TOKEN = getenv("TB_DEVICE_TOKEN");
const char* SERVER = getenv("TB_SERVER");
```

Set in your IDE or PlatformIO:
```ini
[env:esp32]
build_flags = 
    -D TB_TOKEN="${sysenv.TB_DEVICE_TOKEN}"
    -D TB_SERVER="${sysenv.TB_SERVER}"
```

### 2. Use Git Hooks (Prevent Accidents)

Create `.git/hooks/pre-commit`:
```bash
#!/bin/bash
if git diff --cached --name-only | grep -q "config.h"; then
    echo "ERROR: Attempting to commit config.h!"
    echo "This file contains sensitive credentials."
    exit 1
fi
```

Make executable:
```bash
chmod +x .git/hooks/pre-commit
```

### 3. Use GitHub Secret Scanning

- GitHub automatically scans for known secret patterns
- Enable in: Settings → Security → Secret scanning

### 4. Regular Audits

Monthly check:
```bash
git log --all --full-history -- config.h
# Should show NO commits (file always ignored)
```

## 📚 Additional Resources

### Git Security Tools

1. **git-secrets** (AWS)
```bash
brew install git-secrets
git secrets --install
git secrets --register-aws
```

2. **gitleaks**
```bash
brew install gitleaks
gitleaks detect
```

3. **truffleHog**
```bash
pip install truffleHog
trufflehog --regex --entropy=True .
```

### If Secrets Are Leaked

1. **GitHub will notify you** (if detected)
2. **Immediately rotate credentials**
3. **Remove from history** (as shown above)
4. **Monitor for unauthorized access**

## ✅ Final Verification Checklist

Before making repository public:

- [ ] config.h is in .gitignore
- [ ] No hardcoded credentials in .ino files
- [ ] config_template.h has placeholders only
- [ ] README doesn't contain secrets
- [ ] Searched repository for sensitive strings
- [ ] Tested clone on different machine (should need config.h)
- [ ] All real credentials are rotated (if previously exposed)

## 🎓 Understanding the Safe Architecture

### Your Code Structure:

```
mushroom_controller_github.ino
├── #include "config.h"      ← File exists locally only
├── Uses: TOKEN              ← Value from config.h
├── Uses: THINGSBOARD_SERVER ← Value from config.h
└── No hardcoded secrets!    ← Safe for GitHub

config.h (LOCAL ONLY, NOT IN GIT)
├── const char* TOKEN = "FvyMKVz66JbHGHWy3Ca5"
└── const char* THINGSBOARD_SERVER = "167.99.69.166"

config_template.h (IN GIT)
├── const char* TOKEN = "YOUR_DEVICE_TOKEN_HERE"
└── const char* THINGSBOARD_SERVER = "YOUR_SERVER_IP_HERE"
```

### How Others Use Your Code:

1. Clone repository
2. Copy: `cp config_template.h config.h`
3. Edit config.h with their credentials
4. Upload to their ESP32
5. Everyone has their own secrets!

## 🆘 Emergency Contact

If you accidentally committed secrets:

1. **DO NOT PANIC**
2. **DO NOT DELETE REPOSITORY** (evidence needed)
3. **Immediately rotate credentials**
4. **Follow "Already Pushed" steps above**
5. **Monitor ThingsBoard device logs**

## 💡 Remember

> "Once something is on the internet, assume it's there forever."

Even if you remove credentials from GitHub:
- They exist in Git history
- They may be cached by search engines
- Others may have cloned your repo

**Solution:** Always rotate exposed credentials immediately!

---

**Need Help?**
- GitHub Security: https://docs.github.com/en/code-security
- Git Secrets Tools: https://github.com/awslabs/git-secrets
