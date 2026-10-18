<!doctype html>
<html lang="en">
	<head>
		<meta charset="utf-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1" />

		<script>if(typeof window.process==="undefined"){window.process={env:{},browser:true,version:"v20.0.0",nextTick:function(fn){var a=[].slice.call(arguments,1);return setTimeout(function(){fn.apply(null,a)},0);}}}</script>
		<script>
			// Polyfill for bits-ui scroll event crash (Window.contains is missing)
			if (typeof Window !== "undefined" && !Window.prototype.contains) {
				Window.prototype.contains = function () {
					return false;
				};
			}

			// Prevent ReferenceErrors from Android WebView wrapper callbacks
			if (typeof window !== "undefined") {
				if (!window.onHide) window.onHide = function() { };
				if (!window.onReady) window.onReady = function() { };
			}

			// Run before mode-watcher's module-level storage reads. Only theme keys
			// may fall back to memory; auth and other storage failures must stay visible.
			(function () {
				try {
					if (typeof window === "undefined") return;

					var keys = {
						"mode-watcher-mode": true,
						"mode-watcher-theme": true
					};
					var fallback = Object.create(null);
					var readUnavailable = false;

					function isModeKey(key) {
						return keys[String(key)] === true;
					}

					var storage;
					var accessError;
					try {
						storage = window.localStorage;
					} catch (error) {
						accessError = error;
					}
					if (!storage) {
						accessError = accessError || new Error("localStorage unavailable");
						function unavailable() { throw accessError; }
						// A denied getter cannot be repaired by patching Storage.prototype.
						var memory = {
							getItem: function (key) {
								if (!isModeKey(key)) return unavailable();
								return Object.prototype.hasOwnProperty.call(fallback, key) ? fallback[key] : null;
							},
							setItem: function (key, value) {
								if (!isModeKey(key)) return unavailable();
								fallback[key] = String(value);
							},
							removeItem: function (key) {
								if (!isModeKey(key)) return unavailable();
								delete fallback[key];
							},
							clear: unavailable,
							key: unavailable
						};
						Object.defineProperty(memory, "length", { get: unavailable });
						Object.defineProperty(window, "localStorage", { configurable: true, enumerable: true, value: memory });
						return;
					}
					if (typeof Storage === "undefined") return;
					var proto = Storage.prototype;
					if (proto.__fileKiwiModeStoragePatch) return;
					var getItem = proto.getItem;
					var setItem = proto.setItem;
					var removeItem = proto.removeItem;

					Object.defineProperty(proto, "__fileKiwiModeStoragePatch", { value: true });

					proto.getItem = function (key) {
						if (this !== storage || !isModeKey(key)) return getItem.call(this, key);

						try {
							return Object.prototype.hasOwnProperty.call(fallback, key)
								? fallback[key]
								: getItem.call(this, key);
						} catch {
							readUnavailable = true;
							return Object.prototype.hasOwnProperty.call(fallback, key) ? fallback[key] : null;
						}
					};

					proto.setItem = function (key, value) {
						if (this !== storage || !isModeKey(key)) return setItem.call(this, key, value);

						var normalizedValue = String(value);
						try {
							setItem.call(this, key, normalizedValue);
							if (readUnavailable) fallback[key] = normalizedValue;
							else delete fallback[key];
						} catch {
							fallback[key] = normalizedValue;
						}
					};

					proto.removeItem = function (key) {
						if (this !== storage || !isModeKey(key)) return removeItem.call(this, key);

						delete fallback[key];
						try {
							removeItem.call(this, key);
						} catch {
							fallback[key] = null;
						}
					};
				} catch {}
			})();
		</script>
		<link href="/_app/immutable/entry/start.BxJOZSzP.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/YYaf0mMs.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/C6RessQ6.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Dzff6Fc3.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/hePW80VL.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/mF56JYAf.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BdmYSKho.js" rel="modulepreload">
		<link href="/_app/immutable/entry/app.DUfBo9j_.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/HclGiUj8.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BzSLt2FA.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/u3fpLLXf.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Cx80NDhD.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Cd3c3kq1.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BIGDcUow.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Grb3K01z.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/xihTtKlq.js" rel="modulepreload">
		<link href="/_app/immutable/nodes/0.CBnGGN8l.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DVWZEs2x.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BvvZzR_p.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BCAkO4Kv.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/ByfWq_pw.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/NOlwLf1a.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CmRIn6he.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CxFu-6Qv.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/D1ifSmxI.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/hZtNfgsz.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/QbXOXUvK.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CTFZ2M-N.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Sgj00but.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BQXWFTuh.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Dc5Smvoa.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BI816Zt1.js" rel="modulepreload">
		<link href="/_app/immutable/nodes/2.CoWflwqZ.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CTnF_Y9G.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CV_rzZkv.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/B9P77Vmj.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DmQIudZV.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Ni5MVhIG.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/AedHnI_4.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/RO8FFvsw.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CjO64C-P.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DxdGK6Xj.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CSLkJQ-k.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BDi-fMVY.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BKyO72Jh.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CzvepXYn2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/kLNzjQ_C2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BfY-Txvq2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BaStYaoh2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DAuY_D3n2.js" rel="modulepreload">
		<link href="/_app/immutable/nodes/3.AhVR5Z2H.js" rel="modulepreload">
		<link href="/_app/immutable/nodes/8.BN-UFWp7.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BFjOh2U42.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BaOQ47FQ2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Bp_3sXQL2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/ERIw1Kwf2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BNgE0J3l2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CnalWwgT2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/B1vxFVh22.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Bzl7zOgK2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Ch4DoACT2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BnMkCS2F2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BPgTZPqH2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Cp835fRx2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CDbkByCG.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/C8rQQs-m.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/g7qQnlT1.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/5o828efA.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/B0erI7GI.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BYCsmYXU2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DzMJWtRI2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/4MKz-JTU.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BC0XWlqZ.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DA40PuK1.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/Bv_Msu4y.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CsZ6S2Nu2.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DmF489fm.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/CLFtOwIT.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/powEUtyo.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/BECXotdO.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/C3EiCpPo.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/DhH1JyrF.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/B2pEvrwo.js" rel="modulepreload">
		<link href="/_app/immutable/chunks/C5XLko_S.js" rel="modulepreload">
		<link href="/_app/env.js" rel="modulepreload">
		<!--12qhfyh--><meta name="ahrefs-site-verification" content="95d1128e3f9877be4553a21bf09b82ae49e780a1bebb1d8b83792fcfbc3e5177"/> <meta charset="UTF-8"/> <meta name="viewport" content="width=device-width,initial-scale=1.0"/> <link rel="icon" href="/_app/immutable/assets/logo.CPvgfnY8.svg"/> <link rel="apple-touch-icon" href="/apple-touch-icon.png"/> <meta name="naver-site-verification" content="737534e4286d7fac8dbcbdf26640e1a59b73d82c"/> <link rel="sitemap" href="/sitemap-index.xml"/><!----><!--1funsus--><!--[-1--><!--]--> <!----><script>(function setInitialMode({ defaultMode = "system", themeColors, darkClassNames = ["dark"], lightClassNames = [], defaultTheme = "", modeStorageKey = "mode-watcher-mode", themeStorageKey = "mode-watcher-theme" }) {
	const rootEl = document.documentElement;
	const mode = localStorage.getItem(modeStorageKey) ?? defaultMode;
	const theme = localStorage.getItem(themeStorageKey) ?? defaultTheme;
	const light = mode === "light" || mode === "system" && window.matchMedia("(prefers-color-scheme: light)").matches;
	if (light) {
		if (darkClassNames.length) rootEl.classList.remove(...darkClassNames.filter(Boolean));
		if (lightClassNames.length) rootEl.classList.add(...lightClassNames.filter(Boolean));
	} else {
		if (lightClassNames.length) rootEl.classList.remove(...lightClassNames.filter(Boolean));
		if (darkClassNames.length) rootEl.classList.add(...darkClassNames.filter(Boolean));
	}
	rootEl.style.colorScheme = light ? "light" : "dark";
	if (themeColors) {
		const themeMetaEl = document.querySelector("meta[name=\"theme-color\"]");
		if (themeMetaEl) themeMetaEl.setAttribute("content", mode === "light" ? themeColors.light : themeColors.dark);
	}
	if (theme) {
		rootEl.setAttribute("data-theme", theme);
		localStorage.setItem(themeStorageKey, theme);
	}
	localStorage.setItem(modeStorageKey, mode);
})({"defaultMode":"system","darkClassNames":["dark"],"lightClassNames":[],"defaultTheme":"","modeStorageKey":"mode-watcher-mode","themeStorageKey":"mode-watcher-theme"});</script><!----><!----><!--1xzumtg--><!--[0--><link rel="alternate" hreflang="x-default" href="https://file.kiwi/"/><!--]--> <!--[--><link rel="alternate" hreflang="en" href="https://file.kiwi/"/><link rel="alternate" hreflang="ko" href="https://file.kiwi/ko"/><link rel="alternate" hreflang="ja" href="https://file.kiwi/ja"/><link rel="alternate" hreflang="es" href="https://file.kiwi/es"/><link rel="alternate" hreflang="de" href="https://file.kiwi/de"/><link rel="alternate" hreflang="fr" href="https://file.kiwi/fr"/><link rel="alternate" hreflang="vi" href="https://file.kiwi/vi"/><link rel="alternate" hreflang="zh" href="https://file.kiwi/zh"/><link rel="alternate" hreflang="it" href="https://file.kiwi/it"/><link rel="alternate" hreflang="pt" href="https://file.kiwi/pt"/><link rel="alternate" hreflang="ru" href="https://file.kiwi/ru"/><link rel="alternate" hreflang="id" href="https://file.kiwi/id"/><link rel="alternate" hreflang="tr" href="https://file.kiwi/tr"/><link rel="alternate" hreflang="th" href="https://file.kiwi/th"/><link rel="alternate" hreflang="hi" href="https://file.kiwi/hi"/><!--]--><!----><!--gsrl61--><link rel="canonical" href="https://file.kiwi/"/> <meta name="language" content="en"/> <meta name="description" content="Upload files online for free with no file size limit. Get a shareable link instantly, no signup required, with end-to-end encryption and auto-expiring files."/> <!--[0--><meta name="keywords" content="file sharing, file sharing free, free file sharing, share large files, file transfer, large file upload, send large files, file sharing online, quick file sharing"/><!--]--> <!--[0--><meta name="author" content="file.kiwi"/><!--]--> <meta name="robots" content="index,follow"/> <meta name="url" content="https://file.kiwi/"/> <!--[--><!--]--> <meta property="og:title" content="Free File Transfer, No File Size Limit | file.kiwi"/> <meta property="og:site_name" content="file.kiwi"/> <meta property="og:description" content="Upload files online for free with no file size limit. Get a shareable link instantly, no signup required, with end-to-end encryption and auto-expiring files."/> <meta property="og:url" content="https://file.kiwi/"/> <meta property="og:type" content="website"/> <meta property="og:locale" content="en"/> <!--[-1--><!--]--> <!--[--><!--[0--><meta property="og:image" content="https://file.kiwi/og.png"/><!--]--> <!--[0--><meta property="og:image:secure_url" content="https://file.kiwi/og.png"/><!--]--> <!--[0--><meta property="og:image:type" content="image/png"/><!--]--> <!--[0--><meta property="og:image:width" content="1200"/><!--]--> <!--[0--><meta property="og:image:height" content="630"/><!--]--> <!--[-1--><!--]--><!--]--> <meta name="twitter:description" content="Upload files online for free with no file size limit. Get a shareable link instantly, no signup required, with end-to-end encryption and auto-expiring files."/> <meta name="twitter:title" content="Free File Transfer, No File Size Limit | file.kiwi"/> <!--[-1--><!--]--> <!--[-1--><!--]--> <!--[-1--><!--]--> <!--[-1--><!--]--> <meta name="twitter:card" content="summary_large_image"/> <!--[--><!--]--><!----><!--1cgj44f--><!----><script type="application/ld+json">{
  "@context": "https://schema.org",
  "@type": "WebApplication",
  "url": "https://file.kiwi",
  "name": "file.kiwi",
  "description": "Upload files online for free with no file size limit. Get a shareable link instantly, no signup required, with end-to-end encryption and auto-expiring files.",
  "applicationCategory": "Productivity",
  "operatingSystem": "Mac OS X, Windows, Linux, Android, iOS",
  "offers": {
    "@type": "Offer",
    "price": "0",
    "priceCurrency": "USD"
  },
  "featureList": "https://docs.file.kiwi/en/about/detail/"
}</script><!----><!----><!--1cgj44f--><!----><script type="application/ld+json">{
  "@context": "https://schema.org",
  "@type": "HowTo",
  "name": "How to upload and share large files for free with no file size limit",
  "supply": {
    "@type": "HowToSupply",
    "name": "internet connected device"
  },
  "estimatedCost": {
    "@type": "MonetaryAmount",
    "currency": "USD",
    "value": "0"
  },
  "totalTime": "PT10S",
  "tool": [
    {
      "@type": "HowToTool",
      "name": "internet browser",
      "requiredQuantity": 1
    }
  ],
  "step": [
    {
      "@type": "HowToStep",
      "position": 1,
      "image": "https://docs.file.kiwi/images/image-21.png",
      "name": "Navigate to file.kiwi.",
      "text": "Navigate to file.kiwi.",
      "url": "https://file.kiwi"
    },
    {
      "@type": "HowToStep",
      "position": 2,
      "image": "https://docs.file.kiwi/images/image-23.png",
      "name": "Select files",
      "text": "Select files",
      "url": "https://file.kiwi"
    },
    {
      "@type": "HowToStep",
      "position": 3,
      "image": "https://docs.file.kiwi/images/image-22.png",
      "name": "Copy link",
      "text": "Copy link",
      "url": "https://file.kiwi"
    }
  ]
}</script><!----><!----><!--1cgj44f--><!----><script type="application/ld+json">{
  "@context": "https://schema.org",
  "@type": "FAQPage",
  "mainEntity": [
    {
      "@type": "Question",
      "name": "What is file sharing?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "File sharing is the act of distributing digital files — documents, videos, photos, archives — between people or devices over the internet. Modern cloud-based file sharing replaces USB drives, email attachments, and FTP with a one-link flow: you upload once, and anyone with the link can download directly without installing software."
      }
    },
    {
      "@type": "Question",
      "name": "How does file sharing work on file.kiwi?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "When you pick a file, your browser encrypts it with 128-bit AES-GCM before anything is sent. The encrypted blob is stored on our server; the decryption key lives only inside the share link. When the recipient opens the link, their browser fetches the blob and decrypts it locally. Your original file never sits unencrypted on our disk."
      }
    },
    {
      "@type": "Question",
      "name": "Is file sharing on file.kiwi secure?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Yes. Files are end-to-end encrypted in your browser with 128-bit AES-GCM before upload. The decryption key never leaves the share link, so nobody — including us — can read the content without it. Encrypted files are auto-deleted from the server after 90 hours by default."
      }
    },
    {
      "@type": "Question",
      "name": "What is the main benefit of cloud-based file sharing?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The main benefit is decoupling sender and receiver in both time and place. A recipient in a different time zone, on a different OS, behind a different network can still download on their own schedule — without installing software, pairing devices, or hitting the size limits of email or chat apps."
      }
    },
    {
      "@type": "Question",
      "name": "Is free file sharing really safe?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Not always. Many free services are free because they monetize metadata, sell access, or skip encryption. file.kiwi is the opposite: files are encrypted client-side before upload, the decryption key never reaches our server, and the encrypted blob is deleted after 90 hours by default."
      }
    },
    {
      "@type": "Question",
      "name": "Does the recipient also need to install it?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "file.kiwi is essentially a webpage. With just a web browser, you can use all the features. Neither the sender nor the recipient needs to install anything."
      }
    },
    {
      "@type": "Question",
      "name": "How can this service be free — without even ads?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "file.kiwi is free by default, but there are premium benefits."
      }
    },
    {
      "@type": "Question",
      "name": "Is there really no file size limit for file sharing?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "file.kiwi doesn't impose any file size limits of its own. In practice, the maximum size depends on your device and browser. Desktops handle very large files more reliably than phones — see Help & Support for device-specific limits."
      }
    },
    {
      "@type": "Question",
      "name": "Can it be used on mobile devices (iOS or Android)?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Yes, though very large files may be constrained by your device's performance. See related help"
      }
    },
    {
      "@type": "Question",
      "name": "Can I download files directly to an external hard drive?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Yes. On a PC, you can save downloads directly to an external drive so they do not use space on your C: drive. See the Large File Download Guide."
      }
    }
  ]
}</script><!----><!----><title>Free File Transfer, No File Size Limit | file.kiwi</title>
		<link href="/_app/immutable/assets/0.DLEIVPzX.css" rel="stylesheet">
		<link href="/_app/immutable/assets/8.Dkn6o73t.css" rel="stylesheet">
		<link href="/_app/immutable/assets/uploadbox.BFw-s4zU.css" rel="stylesheet">
	</head>
	<body data-sveltekit-preload-data="hover">
		<div style="display: contents"><!--[--><!--[0--><!--[--><section aria-label="Notifications alt+T" tabindex="-1" aria-live="polite" aria-relevant="additions text" aria-atomic="false"><!--[-1--><!--]--></section><!----> <!--[-1--><!--]--><!----> <div class="body-large light" style="background-color:var(--md-sys-color-background); min-height: 100vh;"><!--[0--><!--[--><div class="flex w-full flex-col overflow-x-hidden"><a href="#content" class="sr-only fixed top-4 left-4 z-[60] rounded bg-background px-3 py-2 text-sm font-medium text-foreground shadow-[var(--shadow-soft)] focus:not-sr-only focus:ring-2 focus:ring-ring focus:outline-none">Skip to content</a> <header class="sticky top-0 z-50 bg-background/80 backdrop-blur supports-backdrop-filter:bg-background/60"><div class="mx-auto flex max-w-7xl items-center justify-between gap-4 px-4 py-6"><a href="/" class="h-auto self-center text-base font-bold whitespace-nowrap text-foreground md:text-xl"><img src="/logo.svg" alt="" class="inline size-8"/><span class="ml-2 leading-none text-accent-foreground/70">file.kiwi</span></a> <nav class="absolute left-1/2 hidden -translate-x-1/2 items-center gap-2 md:flex"><!--[--><!--[0--><div class="group relative"><button type="button" class="h-9 rounded px-3 text-base font-medium text-foreground capitalize hover:bg-accent hover:text-accent-foreground">features</button> <ul class="invisible absolute left-0 z-50 grid w-max max-w-[min(22rem,calc(100vw-2rem))] min-w-64 gap-1 rounded-xl border border-border bg-popover p-2 text-popover-foreground opacity-0 shadow-[var(--shadow-soft)] transition group-focus-within:visible group-focus-within:opacity-100 group-hover:visible group-hover:opacity-100"><!--[--><li><a href="/howtos/transfer-large-file-for-free" class="block rounded px-3 py-2 text-sm leading-snug font-medium whitespace-normal text-popover-foreground hover:bg-accent hover:text-accent-foreground">Transfer and share large files</a></li><li><a href="/howtos/realtime-shared-storage-for-collaboration" class="block rounded px-3 py-2 text-sm leading-snug font-medium whitespace-normal text-popover-foreground hover:bg-accent hover:text-accent-foreground">Real-time shared storage for collaboration</a></li><li><a href="/howtos/collect-large-files-with-one-link" class="block rounded px-3 py-2 text-sm leading-snug font-medium whitespace-normal text-popover-foreground hover:bg-accent hover:text-accent-foreground">Collect large files with one link</a></li><!--]--></ul></div><!--]--><!--[-1--><a href="/blog" class="inline-flex h-9 items-center rounded px-3 text-base font-medium text-foreground capitalize hover:bg-accent hover:text-accent-foreground">Blog</a><!--]--><!--[-1--><a href="/howtos/apps" class="inline-flex h-9 items-center rounded px-3 text-base font-medium text-foreground capitalize hover:bg-accent hover:text-accent-foreground">Apps</a><!--]--><!--[-1--><a href="/price" class="inline-flex h-9 items-center rounded px-3 text-base font-medium text-foreground capitalize hover:bg-accent hover:text-accent-foreground">Pricing</a><!--]--><!--]--></nav> <details class="group relative mr-auto md:hidden"><summary class="flex size-9 cursor-pointer list-none items-center justify-center rounded border border-border bg-background text-foreground hover:bg-accent [&amp;::-webkit-details-marker]:hidden" aria-label="Open menu"><svg class="size-5" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M4 6h16M4 12h16M4 18h16"></path></svg></summary> <div class="absolute left-0 z-50 mt-2 w-72 rounded-xl border border-border bg-background p-3 shadow-[var(--shadow-soft)]"><!--[--><!--[0--><div class="mb-3"><div class="mb-1 px-2 text-sm font-semibold text-foreground">features</div> <div class="flex flex-col gap-1"><!--[--><a href="/howtos/transfer-large-file-for-free" class="rounded px-3 py-2 text-sm text-foreground hover:bg-accent hover:text-accent-foreground">Transfer and share large files</a><a href="/howtos/realtime-shared-storage-for-collaboration" class="rounded px-3 py-2 text-sm text-foreground hover:bg-accent hover:text-accent-foreground">Real-time shared storage for collaboration</a><a href="/howtos/collect-large-files-with-one-link" class="rounded px-3 py-2 text-sm text-foreground hover:bg-accent hover:text-accent-foreground">Collect large files with one link</a><!--]--></div></div><!--]--><!--[-1--><a href="/blog" class="block rounded px-3 py-2 text-sm font-medium text-foreground hover:bg-accent hover:text-accent-foreground">Blog</a><!--]--><!--[-1--><a href="/howtos/apps" class="block rounded px-3 py-2 text-sm font-medium text-foreground hover:bg-accent hover:text-accent-foreground">Apps</a><!--]--><!--[-1--><a href="/price" class="block rounded px-3 py-2 text-sm font-medium text-foreground hover:bg-accent hover:text-accent-foreground">Pricing</a><!--]--><!--]--></div></details> <div class="flex items-center gap-2"><!--[--><!--[--><!--[--><!--[--><!--[--><!--[--><!--$s1--><!--[-1--><button data-slot="tooltip-trigger" aria-label="Change language" aria-expanded="false" popovertarget="lang-popover" class="inline-flex h-9 w-9 items-center justify-center rounded border border-border bg-background text-foreground outline-none hover:bg-accent focus-visible:ring-[3px] focus-visible:ring-ring/50" id="bits-s1" data-state="closed" data-disabled="" data-delay-duration="0" data-tooltip-trigger="" type="button"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-globe size-4"><!--[--><!----><circle cx="12" cy="12" r="10"><!----></circle><!----><!----><path d="M12 2a14.5 14.5 0 0 0 0 20 14.5 14.5 0 0 0 0-20"><!----></path><!----><!----><path d="M2 12h20"><!----></path><!----><!--]--><!----></svg><!----> <span class="sr-only">Change language</span><!----></button><!--]--><!--]--><!--]--> <!--[-1--><!--]--><!----><!----><!--]--><!--]--><!----><!--]--><!--]--> <div id="lang-popover" popover="auto" class="[inset:auto] m-0 max-h-[60vh] min-w-36 overflow-y-auto rounded-md border border-border bg-popover p-1 text-popover-foreground shadow-[var(--shadow-soft)] hidden"><!--[--><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground" aria-current="true"><span>English</span> <!--[0--><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-check size-4 text-primary"><!--[--><!----><path d="M20 6 9 17l-5-5"><!----></path><!----><!--]--><!----></svg><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>한국어</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>日本語</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Español</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Deutsch</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Français</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Tiếng Việt</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>中文</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Italiano</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Português</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Русский</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Bahasa Indonesia</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>Türkçe</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>ไทย</span> <!--[-1--><!--]--></button><button type="button" class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-1.5 text-left text-sm hover:bg-accent hover:text-accent-foreground"><span>हिन्दी</span> <!--[-1--><!--]--></button><!--]--></div><!----> <button type="button" class="relative inline-flex size-9 shrink-0 items-center justify-center gap-2 overflow-hidden rounded-md text-sm font-medium whitespace-nowrap outline-hidden transition-all select-none focus-visible:border-ring focus-visible:ring-[3px] focus-visible:ring-ring/50 disabled:pointer-events-none disabled:opacity-50 border bg-background hover:bg-accent hover:text-accent-foreground"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-sun h-[1.2rem] w-[1.2rem] scale-100 rotate-0 transition-all! dark:scale-0 dark:-rotate-90"><!--[--><!----><circle cx="12" cy="12" r="4"><!----></circle><!----><!----><path d="M12 2v2"><!----></path><!----><!----><path d="M12 20v2"><!----></path><!----><!----><path d="m4.93 4.93 1.41 1.41"><!----></path><!----><!----><path d="m17.66 17.66 1.41 1.41"><!----></path><!----><!----><path d="M2 12h2"><!----></path><!----><!----><path d="M20 12h2"><!----></path><!----><!----><path d="m6.34 17.66-1.41 1.41"><!----></path><!----><!----><path d="m19.07 4.93-1.41 1.41"><!----></path><!----><!--]--><!----></svg><!----> <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-moon absolute h-[1.2rem] w-[1.2rem] scale-0 rotate-90 transition-all! dark:scale-100 dark:rotate-0"><!--[--><!----><path d="M20.985 12.486a9 9 0 1 1-9.473-9.472c.405-.022.617.46.402.803a6 6 0 0 0 8.268 8.268c.344-.215.825-.004.803.401"><!----></path><!----><!--]--><!----></svg><!----> <span class="sr-only">Switch theme</span></button><!----> <a href="/login" data-sveltekit-reload="" class="inline-flex h-9 items-center justify-center rounded-md bg-secondary px-4 py-2 text-sm font-medium whitespace-nowrap text-secondary-foreground hover:bg-secondary/80">Sign up</a></div></div></header> <main id="content" tabindex="-1" class="grow focus:outline-none"><div class="mx-auto max-w-7xl px-4"><!--[0--><!--[--><!--[--><!----> <!----> <!----> <!----> <div id="intro" data-mode="init" style="min-height: 85vh" class="container mx-auto flex max-w-lg flex-col justify-center text-center md:max-w-2xl"><!--[-1--><!--]--> <div><div id="lhead"><h1 class="mb-6 text-center text-3xl leading-tight font-bold tracking-tight text-foreground md:text-4xl lg:text-5xl"><!--[--><!--[2--><!--[0--><span class="leading-normal break-keep">Upload Files Online for Free</span><br/><!----><!--]--><!--]--><!--[2--><!--[1--><span class="relative mt-1 bg-clip-text break-keep">No File Size Limit</span><!----><!--]--><!--]--><!--]--><!----></h1></div> <div id="upzone" style="max-height: 45vh;" class="relative flex w-full grow flex-col items-center text-center"><div class="animate-fade-in-up z-20 w-full"><span data-mode="init" id="upbtn" class="relative w-full space-y-4 md:w-auto"><p class="mx-auto max-w-xl text-base text-balance md:text-lg">file.kiwi is a real-time web folder optimized for fast file transfer, sharing, and collection.</p> <div class="w-full" role="presentation"><p class="text-sm font-medium text-balance break-keep text-muted-foreground md:text-base"><!--[-1-->No file size limit. No ads.
										 
										Encrypted files.<!--]--></p> <div class="relative mx-auto !mt-6 w-fit rounded-md border bg-foreground/10 p-[1px]"><button data-upload-more-trigger="" id="main-upload-button" type="button" aria-expanded="false" aria-haspopup="true" aria-controls="upload-more-actions" aria-describedby="upload-more-hint" class="long-press-grow inline-flex h-max! w-max! shrink-0 items-center justify-center gap-2 overflow-hidden rounded bg-primary p-0! text-lg font-medium whitespace-nowrap text-primary-foreground transition-all select-none hover:bg-primary/90 focus-visible:ring-[3px] focus-visible:ring-ring/50 focus-visible:outline-hidden disabled:pointer-events-none disabled:opacity-50 svelte-1l4ify7"><span class="relative flex cursor-pointer items-center gap-3 px-6! py-3! font-bold capitalize"><span class="upbtn-normal contents"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-package-plus size-6"><!--[--><!----><path d="M12 22V12"><!----></path><!----><!----><path d="M16 17h6"><!----></path><!----><!----><path d="M19 14v6"><!----></path><!----><!----><path d="M21 10.535V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.729l7 4a2 2 0 0 0 2 .001l1.675-.955"><!----></path><!----><!----><path d="M3.29 7 12 12l8.71-5"><!----></path><!----><!----><path d="m7.5 4.27 8.997 5.148"><!----></path><!----><!--]--><!----></svg><!----> Select files</span> <span class="upbtn-loading hidden"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-loader-circle size-6 animate-spin"><!--[--><!----><path d="M21 12a9 9 0 1 1-6.219-8.56"><!----></path><!----><!--]--><!----></svg><!----></span></span></button> <input data-upload-more-trigger="" type="file" id="inputfile-s85lnb379xf" class="absolute inset-0 z-10 h-full w-full cursor-pointer opacity-0" aria-label="Select files" multiple=""/><!----></div> <span id="upload-more-hint" class="sr-only">Hold for 2 seconds or press Arrow Down to show more upload actions.</span> <input type="file" accept="image/*" capture="environment" data-native-capture="photo" class="sr-only" tabindex="-1" aria-hidden="true"/> <input type="file" accept="video/*" capture="environment" data-native-capture="video" class="sr-only" tabindex="-1" aria-hidden="true"/> <input type="file" accept="audio/*" capture="user" data-native-capture="audio" class="sr-only" tabindex="-1" aria-hidden="true"/> <!--[-1--><!--]--> <!--[--><!--[--><!--[--><!--[--><!--[--><!--[-1--><!--]--><!--]--><!--]--><!--]--><!----><!--]--><!--]--> <!--[--><!--[--><!--[--><!--[--><!--[--><!--[-1--><!--]--><!--]--><!--]--><!--]--><!----><!--]--><!--]--><!----></div> <div class="mt-3 block text-sm text-muted-foreground"><div class="relative mx-auto flex max-w-xs items-center px-16 py-2"><div class="grow border-t border-border"></div> <span class="mx-4 shrink text-muted-foreground">or</span> <div class="grow border-t border-border"></div></div> <a href="/start?ext=new" target="_blank" rel="noopener noreferrer" class="inline-flex h-9 items-center gap-2 rounded border border-border bg-background px-3 text-sm font-medium text-foreground hover:bg-accent hover:text-accent-foreground focus-visible:ring-[3px] focus-visible:ring-ring/50 focus-visible:outline-hidden"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" class="lucide-icon lucide lucide-folder-plus size-5"><!--[--><!----><path d="M12 10v6"><!----></path><!----><!----><path d="M9 13h6"><!----></path><!----><!----><path d="M20 20a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-7.9a2 2 0 0 1-1.69-.9L9.6 3.9A2 2 0 0 0 7.93 3H4a2 2 0 0 0-2 2v13a2 2 0 0 0 2 2Z"><!----></path><!----><!--]--><!----></svg><!----> <span>New Shared Web Folder</span></a> <p class="mt-3 text-sm text-muted-foreground">Exchange files with anyone—no sign-up required.</p> <!--[-1--><p class="mx-auto mt-4 min-h-10 w-full text-xs text-muted-foreground sm:max-w-md">An anonymous ID is used for file deletion.
										  <a href="/legal/privacy" class="break-keep underline hover:text-foreground">Privacy Policy</a></p><!--]--></div></span></div></div> <!--[-1--><!--]--><!----> <div class="button-container pt-6 hidden"><button type="button" class="inline-flex size-9 items-center justify-center rounded-full bg-secondary text-secondary-foreground hover:bg-secondary/80 focus-visible:ring-[3px] focus-visible:ring-ring/50 focus-visible:outline-hidden"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg> <span class="sr-only">Close</span></button></div></div><!----><!----></div> <div><section class="space-y-16 py-24 text-base"><div class="items-center gap-8 py-0 md:grid md:grid-cols-2 xl:gap-16"><div style="line-height: 1" class="mb-3 block h-full text-center md:mb-0"><span class="mx-auto inline-block max-w-md px-6 text-center select-none lg:max-w-lg lg:p-4"><img src="/_app/immutable/assets/i1.BHDtr_7b.png" alt="" width="500" height="500" loading="lazy" decoding="async"/></span></div> <div class="mt-4 md:mt-0 md:pr-8 lg:pr-16 xl:pr-24"><h2 class="mb-4 border-0 border-border text-xl leading-tight font-semibold tracking-tight text-foreground md:text-2xl lg:text-3xl">Unlimited File Sharing — Any Size</h2> <p class="mb-6 font-light text-muted-foreground md:text-lg"><!---->Share without size limits.<!----></p> <ul class="mt-[calc(1rem*(1-var(--tw-space-y-reverse)))] mb-[calc(1rem*var(--tw-space-y-reverse))] space-y-3"><!--[--><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">No Sign-up Required: Use it immediately without registration.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">No ads: Nothing to watch or click through.</span></li><!--]--></ul></div></div><!----> <div class="items-center gap-8 py-0 md:grid md:grid-cols-2 xl:gap-16"><div style="line-height: 1" class="mb-3 block h-full text-center md:mb-0"><span class="mx-auto inline-block max-w-md px-6 text-center select-none lg:max-w-lg lg:p-4"><img src="/_app/immutable/assets/i2.BFe5v-yh.png" alt="" width="500" height="500" loading="lazy" decoding="async"/></span></div> <div class="mt-4 md:mt-0 md:pr-8 lg:pr-16 xl:pr-24"><h2 class="mb-4 border-0 border-border text-xl leading-tight font-semibold tracking-tight text-foreground md:text-2xl lg:text-3xl">Fail-safe File Sharing</h2> <p class="mb-6 font-light text-muted-foreground md:text-lg"><!---->file.kiwi provides recovery features for when things go wrong. Even if the browser window closes mid-transfer, you can pick up right where you left off.<!----></p> <ul class="mt-[calc(1rem*(1-var(--tw-space-y-reverse)))] mb-[calc(1rem*var(--tw-space-y-reverse))] space-y-3"><!--[--><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">Resume download: Allows you to download from the point where it was interrupted.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">Resume upload: You can continue uploading from where it stopped.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">No Installation Required: Nothing extra to install or set up.</span></li><!--]--></ul></div></div><!----> <div class="items-center gap-8 py-0 md:grid md:grid-cols-2 xl:gap-16"><div style="line-height: 1" class="mb-3 block h-full text-center md:mb-0"><span class="mx-auto inline-block max-w-md px-6 text-center select-none lg:max-w-lg lg:p-4"><img src="/_app/immutable/assets/i4.BG0K6bik.png" alt="" width="500" height="500" loading="lazy" decoding="async"/></span></div> <div class="mt-4 md:mt-0 md:pr-8 lg:pr-16 xl:pr-24"><h2 class="mb-4 border-0 border-border text-xl leading-tight font-semibold tracking-tight text-foreground md:text-2xl lg:text-3xl">Flexible File Sharing with Web Folders</h2> <p class="mb-6 font-light text-muted-foreground md:text-lg"><!---->A Web Folder is a dedicated internet storage space for sharing.<!----></p> <ul class="mt-[calc(1rem*(1-var(--tw-space-y-reverse)))] mb-[calc(1rem*var(--tw-space-y-reverse))] space-y-3"><!--[--><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">Each Web Folder has its own unique link (URL) that makes it easy to share.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">Works with Outlook and corporate email systems that limit attachment sizes, as well as Gmail, social media, and blogs.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">For light use, jump right in — no signup needed.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">Sign up for precise control over read/write permissions.</span></li><!--]--></ul></div></div><!----> <div class="items-center gap-8 py-0 md:grid md:grid-cols-2 xl:gap-16"><div style="line-height: 1" class="mb-3 block h-full text-center md:mb-0"><span class="mx-auto inline-block max-w-md px-6 text-center select-none lg:max-w-lg lg:p-4"><img src="/_app/immutable/assets/i3.ESoT-wss.png" alt="" width="500" height="500" loading="lazy" decoding="async"/></span></div> <div class="mt-4 md:mt-0 md:pr-8 lg:pr-16 xl:pr-24"><h2 class="mb-4 border-0 border-border text-xl leading-tight font-semibold tracking-tight text-foreground md:text-2xl lg:text-3xl">Encrypted File Sharing</h2> <p class="mb-6 font-light text-muted-foreground md:text-lg"><!---->Your original file never leaves your device unencrypted.<!----></p> <ul class="mt-[calc(1rem*(1-var(--tw-space-y-reverse)))] mb-[calc(1rem*var(--tw-space-y-reverse))] space-y-3"><!--[--><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">No one can see what's inside — files are encrypted before upload, and neither the original nor the decryption key is ever stored.</span></li><li class="flex items-start justify-start gap-2.5"><div class="mt-[3px] inline-flex shrink-0 items-center justify-center rounded-full bg-primary p-0.5 text-primary-foreground"><svg aria-hidden="true" class="h-3.5 w-3.5" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor"><path fill-rule="evenodd" d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z" clip-rule="evenodd"></path></svg></div> <span class="text-base leading-6 font-medium text-foreground">No one can search, view, or censor your files.</span></li><!--]--></ul></div></div><!----></section> <section class="content-deferred py-24"><div><div class="mb-12 flex w-full flex-col text-center"><h2 class="mb-4 text-3xl font-semibold tracking-tight text-foreground md:text-5xl">Powerful File Sharing Features</h2> <p class="mx-auto mt-1 max-w-xl text-base text-muted-foreground md:text-lg"></p></div> <div class="grid grid-cols-1 lg:grid-cols-2 xl:grid-cols-3"><div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Seamless transfers.</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">Nothing stands in the way of uploading your files — just select them, and the upload begins instantly. We don't limit file size: 10 GB or 500 GB, it makes no difference. You can upload all types of files, including videos, documents, and images. No ads to watch, no signup walls, no waiting queues, no throttled uploads based on file size. Try sharing even your largest files in one go, without splitting them.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Delivered securely without storing the original — then destroyed completely.</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">We use end-to-end 128-bit AES-GCM encryption without storing the original file or decryption key on the server. File names are also encrypted, so files cannot be read without the shared link. Files are permanently deleted when their validity period ends. Web Folder+ owners can use folder capacity to extend the file validity period in 7-day increments.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Convenient Shared Folders and Batch Transfers</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">Because you share an entire folder (a Web Folder), you can deliver multiple files at once. Just drag and drop — like moving files into any folder — and the upload starts immediately. Even large batches of files can be uploaded in one go.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Real-time Sharing</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">You can share as soon as the upload starts, meaning you don't have to wait for the upload to reach 100% to share the file. This can significantly reduce the sharing time, especially for large files. Whether the file size is 50GB or 100GB, sharing starts in about 10 seconds. Furthermore, if the file is an MP4, it can be played immediately, resulting in zero waiting time for both the sender and the receiver.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Simple Link Sharing</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">Web Folders created on file.kiwi have a unique link address (URL). Send the link and sharing begins instantly. You can also generate a QR code, scan it on another device, or share the link with a single click. Alternatively, copy the link and paste it into an email or chat message.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Convenient two-way sharing for collaboration</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">Beyond simple transfers, a Web Folder can also act as a shared file box where multiple people can upload and download files. You can also allow uploads while limiting downloads to the Web Folder owner. This turns it into a Collection Web Folder, like a mailbox or drop-off point. It's especially convenient for receiving files from people who aren't familiar with file transfers — all they need is the link.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Resume download / Resume upload support</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">When uploading or downloading files, network errors can occur. Starting over each time is a waste of time, and for large files, it may not be feasible at all. file.kiwi internally splits file transfers and allows you to resume from where they left off, rather than starting from the beginning.</p></div> <div class="border-l-2 border-border px-6 py-5"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Apps and tools for every workflow</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">Beyond the browser, file.kiwi ships as a Windows app (right-click from File Explorer), a Chrome extension (right-click inside any text input), a CLI (`npx @file-kiwi/node`), and an MCP server that lets AI tools like Claude or Cursor upload and hand back a download link on command. Pick the integration that fits your workflow.</p> <a class="inline-flex items-center text-primary" href="/howtos/apps">Try It Out <svg fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" class="ml-2 h-4 w-4" viewBox="0 0 24 24"><path d="M5 12h14M12 5l7 7-7 7"></path></svg></a></div></div> <div class="mt-24 mb-12 flex w-full flex-col text-center"><h2 class="mb-4 text-3xl font-semibold tracking-tight text-foreground sm:text-5xl">File Sharing for Various Uses</h2></div> <div class="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3"><div class="border-l-2 border-border px-6 py-5" style="break-inside: avoid;"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">When sending large files as email attachments</h2> <p class="mb-3 text-base leading-7 text-muted-foreground" style="break-inside: avoid;">file.kiwi comes in handy when sending large files via email. Just put the file in the Web Folder and insert the link into the body of the email. Since the original file is not stored on an external service, it's safe to use even at security-conscious companies.</p></div> <div class="border-l-2 border-border px-6 py-5" style="break-inside: avoid;"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Share original photos/videos with friends</h2> <p class="mb-3 text-base leading-7 text-muted-foreground">Photos and videos from trips and parties with friends are meant to be shared. While there are many social media platforms, sharing original files is not always easy, and free options for sharing large files can be scarce. file.kiwi can serve as a shared folder where participants can upload files collaboratively without any sign-up requirements or storage limits. You can preview files on the web and select the ones you want to download all at once.</p></div> <div class="border-l-2 border-border px-6 py-5" style="break-inside: avoid;"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">When you need to receive large files from someone who isn't tech-savvy</h2> <p class="mb-3 text-base leading-7 text-muted-foreground" style="break-inside: avoid;">Sending large files isn't easy for everyone. It can be difficult for those not familiar with things like splitting and compressing files or using cloud services, and for the recipient, walking someone through it over the phone is just as painful. file.kiwi solves this by simply sharing a Web Folder link. File uploads proceed quickly and intuitively without file size restrictions or other conditions. <a class="inline-flex items-center text-primary" target="_blank" rel="noopener noreferrer" href="/blog/9_">Try It Out <svg fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" class="ml-2 h-4 w-4" viewBox="0 0 24 24"><path d="M5 12h14M12 5l7 7-7 7"></path></svg></a></p></div> <div class="border-l-2 border-border px-6 py-5" style="break-inside: avoid;"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">When you need to collect files from many people</h2> <p class="mb-3 text-base leading-7 text-muted-foreground" style="break-inside: avoid;">Receiving files by email from many senders burns hours. A Collection Web Folder acts as a drop-off point — contributors simply click a link and select files, with no account or instructions required. The collector can download everything in one batch. From requesting files to downloading them, everything is handled through a single shared link without exposing an email address.</p></div> <div class="border-l-2 border-border px-6 py-5" style="break-inside: avoid;"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Play videos from your phone on another device</h2> <p class="mb-3 text-base leading-7 text-muted-foreground" style="break-inside: avoid;">When you want to watch MP4 videos from your device on a smart TV or any other big screen, you can play them directly — no login, no pairing, no setup. All you need is a browser on the device you want to play them on. <a class="inline-flex items-center text-primary" target="_blank" rel="noopener noreferrer" href="/blog/2_">Play phone videos fullscreen in a Tesla <svg fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="2" class="ml-2 h-4 w-4" viewBox="0 0 24 24"><path d="M5 12h14M12 5l7 7-7 7"></path></svg></a></p></div> <div class="border-l-2 border-border px-6 py-5" style="break-inside: avoid;"><h2 class="title-font mb-2 text-base font-semibold text-foreground sm:text-lg">Receive files without exposing your personal email</h2> <p class="mb-3 text-base leading-7 text-muted-foreground" style="break-inside: avoid;">Receiving files through forums or social media usually means exposing your personal email address. With a Collection Web Folder link created on file.kiwi, you can receive files safely instead. It's convenient for everyone — senders can upload their files with just a few clicks.</p></div></div></div></section> <section class="content-deferred py-24"><div><h2 class="mb-12 text-center text-3xl font-semibold tracking-tight text-foreground md:text-5xl">Frequently Asked Questions</h2> <div class="grid border-t border-border pt-8 text-left md:grid-cols-2 md:gap-16"><div><div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> What is file sharing?</h3> <p class="text-base leading-7 text-muted-foreground">File sharing is the act of distributing digital files — documents, videos, photos, archives — between people or devices over the internet. Modern cloud-based file sharing replaces USB drives, email attachments, and FTP with a one-link flow: you upload once, and anyone with the link can download directly without installing software.</p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> How does file sharing work on file.kiwi?</h3> <p class="text-base leading-7 text-muted-foreground">When you pick a file, your browser encrypts it with 128-bit AES-GCM before anything is sent. The encrypted blob is stored on our server; the decryption key lives only inside the share link. When the recipient opens the link, their browser fetches the blob and decrypts it locally. Your original file never sits unencrypted on our disk.</p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> What is the main benefit of cloud-based file sharing?</h3> <p class="text-base leading-7 text-muted-foreground">The main benefit is decoupling sender and receiver in both time and place. A recipient in a different time zone, on a different OS, behind a different network can still download on their own schedule — without installing software, pairing devices, or hitting the size limits of email or chat apps.</p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> Does the recipient also need to install it?</h3> <p class="text-base leading-7 text-muted-foreground">file.kiwi is essentially a webpage. With just a web browser, you can use all the features. Neither the sender nor the recipient needs to install anything.</p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> How can this service be free — without even ads?</h3> <p class="text-base leading-7 text-muted-foreground"><!--[--><!--[1-->file.kiwi is free by default, but there are <!--]--><!--[2--><!--[0--><a class="underline" href="/price">premium benefits</a><!----><!--]--><!--]--><!--[1-->.<!--]--><!--]--><!----></p></div></div> <div><div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> Is there really no file size limit for file sharing?</h3> <p class="text-base leading-7 text-muted-foreground"><!--[--><!--[1-->file.kiwi doesn't impose any file size limits of its own. In practice, the maximum size depends on your device and browser. Desktops handle very large files more reliably than phones — see <!--]--><!--[2--><!--[0--><a href="https://docs.file.kiwi/en/about/requirements" class="underline">Help &amp; Support</a><!----><!--]--><!--]--><!--[1--> for device-specific limits.<!--]--><!--]--><!----></p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> Is file sharing on file.kiwi secure?</h3> <p class="text-base leading-7 text-muted-foreground">Yes. Files are end-to-end encrypted in your browser with 128-bit AES-GCM before upload. The decryption key never leaves the share link, so nobody — including us — can read the content without it. Encrypted files are auto-deleted from the server after 90 hours by default.</p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> Is free file sharing really safe?</h3> <p class="text-base leading-7 text-muted-foreground">Not always. Many free services are free because they monetize metadata, sell access, or skip encryption. file.kiwi is the opposite: files are encrypted client-side before upload, the decryption key never reaches our server, and the encrypted blob is deleted after 90 hours by default.</p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> Can I download files directly to an external hard drive?</h3> <p class="text-base leading-7 text-muted-foreground"><!--[--><!--[1-->Yes. On a PC, you can save downloads directly to an external drive so they do not use space on your C: drive. See the <!--]--><!--[2--><!--[0--><a href="/DownloadGuideForHugeFile" class="break-keep underline">Large File Download Guide</a><!----><!--]--><!--]--><!--[1-->.<!--]--><!--]--><!----></p></div> <div class="mb-10"><h3 class="mb-3 flex items-center gap-1.5 text-base font-semibold text-foreground"><svg class="h-5 w-5 shrink-0 text-muted-foreground" fill="currentColor" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zm-8-3a1 1 0 00-.867.5 1 1 0 11-1.731-1A3 3 0 0113 8a3.001 3.001 0 01-2 2.83V11a1 1 0 11-2 0v-1a1 1 0 011-1 1 1 0 100-2zm0 8a1 1 0 100-2 1 1 0 000 2z" clip-rule="evenodd"></path></svg><!----> Can it be used on mobile devices (iOS or Android)?</h3> <p class="text-base leading-7 text-muted-foreground"><!--[--><!--[1-->Yes, though very large files may be constrained by your device's performance. <!--]--><!--[2--><!--[0--><a href="https://docs.file.kiwi/en/about/requirements" class="underline">See related help</a><!----><!--]--><!--]--><!--]--><!----></p></div></div></div></div></section></div><!--]--><!----><!--]--><!--]--><!----></div></main> <footer class="mx-auto mt-5 w-full max-w-7xl px-4 py-6 text-sm"><div class="ko lang flow-root items-start justify-between"><div class="mb-6 flex flex-col space-y-2 md:float-right md:text-right"><a target="_blank" rel="noopener noreferrer" href="/price" class="block capitalize hover:underline">Pricing</a> <a target="_blank" rel="noopener noreferrer" href="https://docs.file.kiwi/en/" class="block capitalize hover:underline">Help</a> <a href="/blog" class="block capitalize hover:underline">Blog</a> <a href="/howtos" class="block capitalize hover:underline">features</a> <a href="/api" class="block capitalize hover:underline">API</a></div> <div class="float-left"><img src="/logo.svg" alt="" class="size-12"/> <canvas class="mt-2 " width="420" height="80"></canvas></div></div> <hr class="my-2 h-px border-none bg-border"/> <div class="my-2 flow-root text-muted-foreground"><div class="float-left flex flex-wrap gap-4"><a href="/legal">Legal</a> <a href="/legal/privacy">Privacy Policy</a> <a href="/legal/terms">Terms of Use</a> <a href="/refund" lang="en" data-sveltekit-reload="">Refund request</a> <a href="https://docs.file.kiwi/en/about/policy/#block" target="_blank" rel="noopener noreferrer">DMCA/Copyright</a></div> <span class="float-right">©file.kiwi 2026 All rights reserved</span></div></footer></div><!--]--><!--]--><!----></div><!--]--><!--]--> <!--[-1--><!--]--><!--]-->
			
			<script>
				{
					__sveltekit_2ygv2v = {
						base: "",
						env: null
					};

					const element = document.currentScript.parentElement;

					import("/_app/env.js").then(({ env }) => {
						__sveltekit_2ygv2v.env = env;

						Promise.all([
							import("/_app/immutable/entry/start.BxJOZSzP.js"),
							import("/_app/immutable/entry/app.DUfBo9j_.js")
						]).then(([kit, app]) => {
							kit.start(app, element, {
									node_ids: [0, 2, 3, 8],
									data: [{type:"data",data:{user:null,hasSessionSnapshot:false,country:"US",currency:"USD",ppp:1,csrfToken:"",hasBrowserSessionCookies:false},uses:{}},{type:"data",data:{docTitles:{"howtos/transfer-large-file-for-free":"Transfer and share large files","howtos/realtime-shared-storage-for-collaboration":"Real-time shared storage for collaboration","howtos/collect-large-files-with-one-link":"Collect large files with one link"}},uses:{}},null,null],
									form: null,
									error: null
								});
						});
					});
				}
			</script>
		</div>
	</body>
</html>
