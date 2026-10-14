# Apple Silicon — အမြန်စတင်နည်း

ဒီ package က source code နဲ့ build scripts ပါ။ အသင့်သွင်းပြီးသုံးနိုင်တဲ့
signed DMG installer မဟုတ်ပါ။

1. Xcode Command Line Tools၊ CMake၊ Git ထည့်ထားပါ။
2. Terminal မှာ ဒီ project folder ကို ဝင်ပါ။
3. `bash scripts/build-apple-silicon.sh` ကို run ပါ။
4. Build အောင်မြင်မှ `bash scripts/install-apple-silicon.sh` ကို run ပါ။
5. Logic Pro ကို ပြန်ဖွင့်ပြီး Plug-in Manager မှာ Matale Tune ကို စစ်ပါ။
6. AU စစ်ရန် `auval -v aufx Mtn1 Mtla` ကို run နိုင်ပါတယ်။

Logic Pro အတွက် AU component၊ VST3 host များအတွက် VST3 ထွက်ပါမယ်။
Pro Tools အတွက် AAX မပါပါ။

“Set Audio On Auto” နှိပ်ပြီး input ထဲကို ၃ စက္ကန့်ခန့် သီဆိုပေးပါ။
Key / Scale ကို ကိုယ်တိုင်ရွေးပါ။ Auto က သီချင်း Key ကို မခန့်မှန်းပါ။
Native editor ကို ဖွင့်ထားချိန်မှာ analysis result ကို host parameters ဆီ
အသုံးချပါတယ်။

48 kHz မှာ engine delay က 42.67 ms ခန့်ရှိပြီး interface / DAW buffer delay
ထပ်ပေါင်းပါမယ်။ Live monitoring မှာ နှောင့်နှေးမှုကို စမ်းသပ်နားထောင်ပါ။
အသံအရည်အသွေးကို စမ်းသပ်ပြီးမှ အရေးကြီးတဲ့ session မှာ သုံးပါ။
