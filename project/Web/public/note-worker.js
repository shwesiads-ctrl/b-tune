import { analyzeVocal, renderNotes, encodeWav, detectKey } from './pitch-editor.js';
self.onmessage=({data})=>{
  try {
    if(data.type==='analyze')self.postMessage({ok:true,result:analyzeVocal(data.samples,data.rate)});
    else if(data.type==='key')self.postMessage({ok:true,result:detectKey(data.frames)});
    else if(data.type==='render'){
      const channels=renderNotes(data.channels,data.rate,data.notes),wav=encodeWav(channels,data.rate);
      self.postMessage({ok:true,result:{channels,wav}},[...channels.map(c=>c.buffer),wav]);
    } else throw new Error('Unknown note worker operation.');
  }catch(error){self.postMessage({ok:false,error:error instanceof Error?error.message:'Audio operation failed.'});}
};
