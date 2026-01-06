// src/app/app.routes.ts
import { Routes } from '@angular/router';
import { CallbackComponent } from './callback/callback';
import { App } from './app'; // Vagy ahogy a fő komponensed osztályát hívják

export const routes: Routes = [
  { path: 'callback', component: CallbackComponent },
  { path: '', component: App }, // Most már nem lesz 'Cannot GET /', mert az App-ot mutatja
  { path: '**', redirectTo: '' } // Minden más ismeretlen utat dobjon vissza a főoldalra
];
