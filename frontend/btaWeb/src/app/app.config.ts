import { ApplicationConfig } from '@angular/core';
import { provideRouter } from '@angular/router';
import { routes } from './app.routes'; // Importáld be a fenti fájlt!

export const appConfig: ApplicationConfig = {
  providers: [
    provideRouter(routes) // Itt adjuk át a térképet az appnak
  ]
};